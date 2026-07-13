#include "EnvTrigger_EventPublisher.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CEnvTrigger_EventPublisher::CEnvTrigger_EventPublisher(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Trigger(pDevice, pContext)
	, m_bPublishOnce{ true }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvTrigger_EventPublisher::CEnvTrigger_EventPublisher(const CEnvTrigger_EventPublisher& Prototype)
	: CEnvObject_Trigger(Prototype)
	, m_strEventTag{ Prototype.m_strEventTag }
	, m_strPayload{ Prototype.m_strPayload }
	, m_strExitEventTag{ Prototype.m_strExitEventTag }
	, m_strExitPayload{ Prototype.m_strExitPayload }
	, m_bPublishOnce{ Prototype.m_bPublishOnce }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvTrigger_EventPublisher::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvTrigger_EventPublisher::Initialize(void* pArg)
{
	return __super::Initialize(pArg);
}

void CEnvTrigger_EventPublisher::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

void CEnvTrigger_EventPublisher::Publish_Event(const _wstring& strEventTag, const _wstring& strPayload, _bool& bPublished)
{
	if (true == m_bPublishOnce && true == bPublished)
		return;

	TRIGGER_EVENT_PAYLOAD Payload{};
	Payload.pTriggerObject = this;
	Payload.strTriggerId = m_strTriggerId;
	Payload.strEventTag = strEventTag;
	Payload.strPayload = strPayload;

	bPublished = true;
	m_pGameInstance_Proxy->Publish(strEventTag, &Payload);

#ifdef _DEBUG
	const wstring strLog = L"[EnvTrigger_EventPublisher] Publish " + strEventTag + L"\n";
	OutputDebugStringW(strLog.c_str());
#endif
}

void CEnvTrigger_EventPublisher::OnTriggerEnter(CCollider* pOther)
{
	if (false == Is_PlayerActivator(pOther))
		return;

	if (true == m_strEventTag.empty())
	{
#ifdef _DEBUG
		OutputDebugStringA("[EnvTrigger_EventPublisher] Event Tag is empty.\n");
#endif
		return;
	}

	Publish_Event(m_strEventTag, m_strPayload, m_bPublished);
}

void CEnvTrigger_EventPublisher::OnTriggerStay(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
}

void CEnvTrigger_EventPublisher::OnTriggerExit(CCollider* pOther)
{
	if (false == Is_PlayerActivator(pOther))
		return;

	if (true == m_strExitEventTag.empty())
		return;

	Publish_Event(m_strExitEventTag, m_strExitPayload, m_bExitPublished);
}

CEnvTrigger_EventPublisher* CEnvTrigger_EventPublisher::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvTrigger_EventPublisher* pInstance = new CEnvTrigger_EventPublisher(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvTrigger_EventPublisher");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvTrigger_EventPublisher::Clone(void* pArg)
{
	CEnvTrigger_EventPublisher* pInstance = new CEnvTrigger_EventPublisher(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvTrigger_EventPublisher");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvTrigger_EventPublisher::Free()
{
	__super::Free();
}

NS_END