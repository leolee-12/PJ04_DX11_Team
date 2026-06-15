#include "EnvObject_Static.h"

#include "GameInstance_Proxy.h"

NS_BEGIN(Client)

CEnvObject_Static::CEnvObject_Static(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Static::CEnvObject_Static(const CEnvObject_Static& Prototype)
	: CEnvObject(Prototype)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvObject_Static::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvObject_Static::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents(m_tDesc.iModelProtoLevel, m_tDesc.strModelProtoTag)))
		return E_FAIL;

	if (FAILED(Ready_PhysicsActor()))
		return E_FAIL;

	return S_OK;
}

void CEnvObject_Static::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (!m_bRenderable || !Has_RenderModel())
	{
		m_bVisible = false;
		return;
	}

	Refresh_WorldBounds();

	Check_Visible();

	if(m_bVisible)
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
	
	if(m_bVisibleShadow)
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CEnvObject_Static::Render_Shadow()
{
	if (!m_bRenderable || nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	size_t n = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < n; ++i)
	{
		if (FAILED(m_pShaderCom->Begin(2)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render((_uint)i)))
			return E_FAIL;
	}
	return S_OK;
}

CGameObject* CEnvObject_Static::Clone(void* pArg)
{
	CEnvObject_Static* pInstance = new CEnvObject_Static(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvObject_Static");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Static::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

CEnvObject_Static* CEnvObject_Static::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvObject_Static* pInstance = new CEnvObject_Static(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvObject_Static");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Static::Free()
{
	__super::Free();
}

NS_END
