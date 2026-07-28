#include "EnvTrigger_Debug.h"

CEnvTrigger_Debug::CEnvTrigger_Debug(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvTrigger_EventPublisher(pDevice, pContext)
{
    m_strProtoTag = PROTOTYPE_TAG;
}

CEnvTrigger_Debug::CEnvTrigger_Debug(const CEnvTrigger_Debug& Prototype)
	: CEnvTrigger_EventPublisher(Prototype)
    , m_strPayload{ Prototype.m_strPayload }
    , m_strExitPayload{ Prototype.m_strExitPayload }
{
    m_strProtoTag = PROTOTYPE_TAG;
}

void* CEnvTrigger_Debug::Get_EnterPayload()
{
	ePayload.pTriggerObject = this;
	ePayload.strTriggerId = m_strTriggerId;
	ePayload.strEventTag = m_strEventTag;
	ePayload.strPayload = m_strPayload;

	return &ePayload;
}

void* CEnvTrigger_Debug::Get_ExitPayload()
{
	ePayload.pTriggerObject = this;
	ePayload.strTriggerId = m_strTriggerId;
	ePayload.strEventTag = m_strExitEventTag;
	ePayload.strPayload = m_strExitPayload;

	return &ePayload;
}

CEnvTrigger_Debug* CEnvTrigger_Debug::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEnvTrigger_Debug* pInstance = new CEnvTrigger_Debug(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEnvTrigger_Debug");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEnvTrigger_Debug::Clone(void* pArg)
{
    CEnvTrigger_Debug* pInstance = new CEnvTrigger_Debug(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CEnvTrigger_Debug");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEnvTrigger_Debug::Free()
{
    __super::Free();
}
