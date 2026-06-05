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

	return S_OK;
}

void CEnvObject_Static::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bRenderable || !Has_RenderModel())
	{
		m_bVisible = false;
		return;
	}

	Refresh_WorldBounds();

	m_bVisible = Is_VisibleInCurrentView();
	if (!m_bVisible)
		return;

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CEnvObject_Static::Render()
{
	return Render_Model();
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
