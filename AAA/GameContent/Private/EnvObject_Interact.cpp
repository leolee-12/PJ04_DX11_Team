#include "EnvObject_Interact.h"

#include "GameInstance_Proxy.h"

NS_BEGIN(Client)

CEnvObject_Interact::CEnvObject_Interact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Interact::CEnvObject_Interact(const CEnvObject_Interact& Prototype)
	: CEnvObject(Prototype)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvObject_Interact::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvObject_Interact::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents(m_tDesc.iModelProtoLevel, m_tDesc.strModelProtoTag)))
		return E_FAIL;

	return S_OK;
}

void CEnvObject_Interact::Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
}

void CEnvObject_Interact::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bRenderable || !Has_RenderModel())
		return;

	Refresh_WorldBounds();
	m_bVisible = true;
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CEnvObject_Interact::Render()
{
	return Render_Model();
}

CGameObject* CEnvObject_Interact::Clone(void* pArg)
{
	CEnvObject_Interact* pInstance = new CEnvObject_Interact(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvObject_Interact");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Interact::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

CEnvObject_Interact* CEnvObject_Interact::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvObject_Interact* pInstance = new CEnvObject_Interact(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvObject_Interact");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Interact::Free()
{
	__super::Free();
}

NS_END
