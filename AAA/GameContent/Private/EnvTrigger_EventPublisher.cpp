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
	, m_strExitEventTag{ Prototype.m_strExitEventTag }
	, m_bPublishOnce{ Prototype.m_bPublishOnce }
{
	m_strProtoTag = PROTOTYPE_TAG;
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

	m_pGameInstance_Proxy->Publish(m_strEventTag, Get_EnterPayload());

#ifdef _DEBUG
	const wstring strLog = L"[EnvTrigger_EventPublisher] Publish " + m_strEventTag + L"\n";
	OutputDebugStringW(strLog.c_str());
#endif

	if (m_bPublishOnce && m_strExitEventTag.empty())
		Set_Active(false);
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

	m_pGameInstance_Proxy->Publish(m_strExitEventTag, Get_ExitPayload());

#ifdef _DEBUG
	const wstring strLog = L"[EnvTrigger_EventPublisher] Publish " + m_strExitEventTag + L"\n";
	OutputDebugStringW(strLog.c_str());
#endif

	if (m_bPublishOnce)
		Set_Active(false);
}

void CEnvTrigger_EventPublisher::Set_Active(_bool b)
{
	if(nullptr != m_pCollider)
		m_pCollider->Set_Enabled(b);

	__super::Set_Active(b);
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