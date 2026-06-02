#include "Effect_Quad.h"

#include "GameInstance.h"

CEffect_Quad::CEffect_Quad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Quad::CEffect_Quad(const CEffect_Quad& Prototype)
    : CEffect_Part(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Quad::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Quad::Initialize(void* pArg)
{
    EFFECT_QUAD_DESC* pDesc = static_cast<EFFECT_QUAD_DESC*>(pArg);

    m_iVIBufferLevel = pDesc->iVIBufferLevel;
    m_wstrVIBufferTag = pDesc->wstrVIBufferTag;

    m_bUseTextureCom = pDesc->bUseTextureCom;
    m_iTextureLevel = pDesc->iTextureLevel;
    m_wstrTextureTag = pDesc->wstrTextureTag;

    m_bCustomShader = pDesc->bCustomShader;
    m_iShaderLevel = pDesc->iShaderLevel;
    m_wstrShaderTag = pDesc->wstrShaderTag;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CEffect_Quad::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_Quad::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CEffect_Quad::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CEffect_Quad::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    const _uint iAlphaBlendPass = 1;
    if (FAILED(m_pShaderCom->Begin(iAlphaBlendPass)))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Quad::Ready_Components()
{
    if(m_bCustomShader == false)
        m_pShaderCom = m_pGameInstance_Proxy->Get_2DShader();
    else
        m_pShaderCom = Add_Component<CShader>(m_iShaderLevel, m_wstrShaderTag, TEXT("Com_Shader"));

    if (m_pShaderCom == nullptr)
        return E_FAIL;

    m_pVIBuffer = Add_Component<CVIBuffer_Rect>(m_iVIBufferLevel, m_wstrVIBufferTag, TEXT("Com_Buffer"));
    if (m_pVIBuffer == nullptr)
        return E_FAIL;

    if (m_bUseTextureCom == true)
    {
        m_pTextureCom = Add_Component<CTexture>(m_iTextureLevel, m_wstrTextureTag, TEXT("Com_Texture"));
        if (m_pTextureCom == nullptr)
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEffect_Quad::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Quad::Bind_ShaderValue()
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

    return S_OK;
}

void CEffect_Quad::Update_EffectPart(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
{

}

void CEffect_Quad::Update_UVScroll(const _float fTimeDelta)
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

    FuncUVScroll(fTimeDelta, m_vTextureUVScroll, m_vTextureOffset, m_vTextureUVSpeed);
}

void CEffect_Quad::Init_PropertyValue()
{
    m_bUseTextureCom = false;
    m_vTextureTiling = { 1.f, 1.f };
    m_vTextureOffset = { 0.f, 0.f };

    m_vTextureUVScroll = false;
    m_vTextureUVSpeed = { 0.f, 0.f };
}

void CEffect_Quad::Free()
{   
    __super::Free();
}