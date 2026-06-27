#include "SkySphere.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CSkySphere::CSkySphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
    , m_iModelIndex(0)
{
}

CSkySphere::CSkySphere(const CSkySphere& Prototype)
	: CGameObject(Prototype)
    , m_iModelIndex(Prototype.m_iModelIndex)
{
}

HRESULT CSkySphere::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkySphere::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pTransformCom->Set_Scale(0.1f, 0.1f, 0.1f);
    
	return S_OK;
}

void CSkySphere::Priority_Update(_float fTimeDelta)
{
}

void CSkySphere::Update(_float fTimeDelta)
{
}

void CSkySphere::Late_Update(_float fTimeDelta)
{
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(m_pGameInstance_Proxy->Get_CamPosition()));
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::PRIORITY, this);
}

HRESULT CSkySphere::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t      iNumMeshes = m_pSphereModels[m_iModelIndex]->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        _uint iPassIdx = 0;
        if (FAILED(m_pSphereModels[m_iModelIndex]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::UNKNOWN, 0)))
            continue;

        //if (FAILED(m_pSphereModels[m_iModelIndex]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", (_uint)i)))
        //    int a = 1;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pSphereModels[m_iModelIndex]->Render((_uint)i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CSkySphere::Ready_Components()
{
	m_pShaderCom = Add_Component<CShader>(Shader_SkySphere.iLevelID, Shader_SkySphere.szProtoTag, TEXT("Com_Shader"));

	m_pSphereModels[ETOUI(SKYTYPE::TEST)] = Add_Component<CModel>(Model_SkyTest.iLevelID, Model_SkyTest.szProtoTag, TEXT("Com_Model"));
	if (m_pSphereModels[ETOUI(SKYTYPE::TEST)] == nullptr)
		return E_FAIL;

	return S_OK;
}

HRESULT CSkySphere::Bind_ShaderResources()
{

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CSkySphere* CSkySphere::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSkySphere* pInstance = new CSkySphere(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CSkySphere");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSkySphere::Clone(void* pArg)
{
    CSkySphere* pInstance = new CSkySphere(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CSkySphere");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSkySphere::Free()
{
    __super::Free();
}

