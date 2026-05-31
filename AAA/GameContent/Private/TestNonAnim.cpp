
#include "TestNonAnim.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CTestNonAnim::CTestNonAnim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{

}

CTestNonAnim::CTestNonAnim(const CTestNonAnim& Prototype)
    : CGameObject(Prototype)
{

}

HRESULT CTestNonAnim::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestNonAnim::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CTestNonAnim::Priority_Update(_float fTimeDelta)
{

}

void CTestNonAnim::Update(_float fTimeDelta)
{
}

void CTestNonAnim::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CTestNonAnim::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t      iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        /*Texture2D g_DiffuseTexture;
        Texture2D g_NormalTexture;
        Texture2D g_UnkownTexture;
        Texture2D g_MRATexture;*/

        _uint iPassIdx = 0;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0)))
            continue;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", (_uint)i, MTEX_TYPE::NORMALS, 0)))
            continue;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", (_uint)i, MTEX_TYPE::METALNESS, 0)))
            continue;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnkownTexture", (_uint)i, MTEX_TYPE::UNKNOWN, 0)))
            continue;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", (_uint)i)))
            int a = 1;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render((_uint)i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CTestNonAnim::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_NonAnim"), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

HRESULT CTestNonAnim::Bind_ShaderResources()
{

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}


CTestNonAnim* CTestNonAnim::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestNonAnim* pInstance = new CTestNonAnim(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTestNonAnim");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestNonAnim::Clone(void* pArg)
{
    CTestNonAnim* pInstance = new CTestNonAnim(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CTestNonAnim");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestNonAnim::Free()
{
    __super::Free();
}
