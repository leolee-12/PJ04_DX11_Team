#include "Material_Object.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CMaterial_Object::CMaterial_Object(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
    , m_vAlbedo(1.f,1.f,1.f,1.f)
    , m_vEmissiveColor(0.f,0.f,0.f,0.f)
    , m_vMRA(0.f,1.f,1.f)
{
}

CMaterial_Object::CMaterial_Object(const CMaterial_Object& Prototype)
    : CGameObject(Prototype)
    , m_vAlbedo(Prototype.m_vAlbedo)
    , m_vEmissiveColor(Prototype.m_vEmissiveColor)
    , m_vMRA(Prototype.m_vMRA)
{
}

HRESULT CMaterial_Object::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CMaterial_Object::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CMaterial_Object::Priority_Update(_float fTimeDelta)
{

}

void CMaterial_Object::Update(_float fTimeDelta)
{
}

void CMaterial_Object::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CMaterial_Object::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t      iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vAlbedo", &m_vAlbedo, sizeof(_float4))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMRA", &m_vMRA, sizeof(_float3))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &m_vEmissiveColor, sizeof(_float4))))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render((_uint)i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CMaterial_Object::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_MtrlTest.iLevelID, Shader_MtrlTest.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_MaterialObject"), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

HRESULT CMaterial_Object::Bind_ShaderResources()
{

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
        return E_FAIL;

    return S_OK;
}


CMaterial_Object* CMaterial_Object::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMaterial_Object* pInstance = new CMaterial_Object(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMaterial_Object");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CMaterial_Object::Clone(void* pArg)
{
    CMaterial_Object* pInstance = new CMaterial_Object(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMaterial_Object");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMaterial_Object::Free()
{
    __super::Free();
}
