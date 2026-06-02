#include "Effect_Mesh.h"

#include "GameInstance.h"

CEffect_Mesh::CEffect_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Mesh::CEffect_Mesh(const CEffect_Mesh& Prototype)
    : CEffect_Part(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Mesh::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Mesh::Initialize(void* pArg)
{
    EFFECT_MESH_DESC* pDesc = static_cast<EFFECT_MESH_DESC*>(pArg);

    // Model
    m_iModelLevel = pDesc->iModelLevel;
    m_wstrModelTag = pDesc->wstrModelTag;
    m_bUseDiffuseTexture = pDesc->bUseDiffuseTexture;
    m_bUseUnknownTexture = pDesc->bUseUnKnownTexture;

    // Texture
    m_bUseTextureCom = pDesc->bUseTextureCom;
    m_iTextureLevel = pDesc->iTextureLevel;
    m_wstrTextureTag = pDesc->wstrTextureTag;

    // Shader
    m_bCustomShader = pDesc->bCustomShader;
    m_iShaderLevel = pDesc->iShaderLevel;
    m_wstrShaderTag = pDesc->wstrShaderTag;


    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CEffect_Mesh::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_Mesh::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    Update_UVScroll(fTimeDelta);
}

void CEffect_Mesh::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CEffect_Mesh::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (m_bUseDiffuseTexture == true)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;
        }

        if (m_bUseUnknownTexture == true)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnknownTexture", i, MTEX_TYPE::UNKNOWN, 0)))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEffect_Mesh::Ready_Components()
{
    if (m_bCustomShader == false)
        m_pShaderCom = m_pGameInstance_Proxy->Get_MeshShader();
    else
        m_pShaderCom = Add_Component<CShader>(m_iShaderLevel, m_wstrShaderTag, TEXT("Com_Shader"));

    m_pModelCom = Add_Component<CModel>(m_iModelLevel, m_wstrModelTag, TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    if (m_bUseTextureCom == true)
    {
        m_pTextureCom = Add_Component<CTexture>(m_iTextureLevel, m_wstrTextureTag, TEXT("Com_Texture"));
        if (m_pTextureCom == nullptr)
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEffect_Mesh::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Mesh::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;
    

    if (m_pTextureCom != nullptr && m_bUseTextureCom == true)
    {
        if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseTexture", &m_bUseTextureCom, sizeof(m_bUseTextureCom))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureTiling", &m_vTextureTiling, sizeof(m_vTextureTiling))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureOffset", &m_vTextureOffset, sizeof(m_vTextureOffset))))
            return E_FAIL;
    }
    else
    {
        _bool bFalse = false;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseTexture", &bFalse, sizeof(bFalse))))
            return E_FAIL;
    }

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseDiffuseTexture", &m_bUseDiffuseTexture, sizeof(m_bUseDiffuseTexture))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vDiffuseTiling", &m_vDiffuseTiling, sizeof(m_vDiffuseTiling))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vDiffuseOffset", &m_vDiffuseOffset, sizeof(m_vDiffuseOffset))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseUnknownTexture", &m_bUseUnknownTexture, sizeof(m_bUseUnknownTexture))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vUnknownTiling", &m_vUnknownTiling, sizeof(m_vUnknownTiling))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vUnknownOffset", &m_vUnknownOffset, sizeof(m_vUnknownOffset))))
        return E_FAIL;

    return S_OK;
}

void CEffect_Mesh::Update_EffectPart(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
{

}

void CEffect_Mesh::Update_UVScroll(const _float fTimeDelta)
{
    auto FuncUVScroll = [](const _float fTimeDelta, const _bool bUpdate, _float2& vUV, const _float2 vSpeed)->void
        {
            if (bUpdate == true)
            {
                vUV.x = vUV.x + vSpeed.x * fTimeDelta;
                vUV.y = vUV.y + vSpeed.y * fTimeDelta;

                vUV.x = fmodf(vUV.x, 1.f);
                vUV.y = fmodf(vUV.y, 1.f);
            }
        };

    FuncUVScroll(fTimeDelta, m_vDiffuseUVScroll, m_vDiffuseOffset, m_vDiffuseUVSpeed);
    FuncUVScroll(fTimeDelta, m_vUnknownUVScroll, m_vUnknownOffset, m_vUnknownUVSpeed);
    FuncUVScroll(fTimeDelta, m_vTextureUVScroll, m_vTextureOffset, m_vTextureUVSpeed);
}

void CEffect_Mesh::Init_PropertyValue()
{
    m_bUseDiffuseTexture = false;
    m_vDiffuseTiling = { 1.f, 1.f };
    m_vDiffuseOffset = { 0.f, 0.f };

    m_vDiffuseUVScroll = false;
    m_vDiffuseUVSpeed = { 0.f, 0.f };


    m_bUseUnknownTexture = false;
    m_vUnknownTiling = { 1.f, 1.f };
    m_vUnknownOffset = { 0.f, 0.f };

    m_vUnknownUVScroll = false;
    m_vUnknownUVSpeed = { 0.f, 0.f };


    m_bUseTextureCom = false;
    m_vTextureTiling = { 1.f, 1.f };
    m_vTextureOffset = { 0.f, 0.f };

    m_vTextureUVScroll = false;
    m_vTextureUVSpeed = { 0.f, 0.f };
}

void CEffect_Mesh::Free()
{
    __super::Free();
}