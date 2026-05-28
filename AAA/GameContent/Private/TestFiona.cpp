
#include "TestFiona.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Navigation.h"

CTestFiona::CTestFiona(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject { pDevice, pContext }    
	, m_iAnimationIndex { 0 }
{
    
}

CTestFiona::CTestFiona(const CTestFiona& Prototype)
    : CGameObject(Prototype)
	, m_iAnimationIndex{ Prototype.m_iAnimationIndex }
{

}

HRESULT CTestFiona::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestFiona::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CTestFiona::Priority_Update(_float fTimeDelta)
{

}

void CTestFiona::Update(_float fTimeDelta)
{   
	m_pModelCom->Set_AnimationIndex(m_iAnimationIndex, true);
    m_pModelCom->Play_Animation(fTimeDelta);
}

void CTestFiona::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CTestFiona::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t      iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", (_uint)i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render((_uint)i)))
            return E_FAIL;
    }   

    // debug
    //m_pNavigationCom->Render();
    
    return S_OK;
}

HRESULT CTestFiona::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_VtxAnimMesh.iLevelID, Shader_VtxAnimMesh.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;


    _float4 vInitPos = {};
    XMStoreFloat4(&vInitPos, m_pTransformCom->Get_State(STATE::POSITION));

    /*CNavigation::NAVIGATION_DESC        NaviDesc{ vInitPos, nullptr };

    m_pNavigationCom = Add_Component<CNavigation>(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Navigation_Lumia"), TEXT("Com_Navigation"), &NaviDesc);
    if (nullptr == m_pNavigationCom)
        return E_FAIL;*/

    return S_OK;
}

HRESULT CTestFiona::Bind_ShaderResources()
{

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
   
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}


CTestFiona* CTestFiona::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestFiona* pInstance = new CTestFiona(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTestFiona");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestFiona::Clone(void* pArg)
{
    CTestFiona* pInstance = new CTestFiona(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CTestFiona");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestFiona::Free()
{
    __super::Free();

    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
}
