#include "EnvTrigger_Generic.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CEnvTrigger_Generic::CEnvTrigger_Generic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Trigger(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvTrigger_Generic::CEnvTrigger_Generic(const CEnvTrigger_Generic& Prototype)
	: CEnvObject_Trigger(Prototype)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

void CEnvTrigger_Generic::OnTriggerEnter(CCollider* pOther)
{
#ifdef _DEBUG
	char szBuf[128];
	sprintf_s(szBuf, "[EnvTrigger_Generic] Enter <- group %u\n", pOther->Get_RegisteredGroup());
	OutputDebugStringA(szBuf);
#else
	UNREFERENCED_PARAMETER(pOther);
#endif
}

void CEnvTrigger_Generic::OnTriggerStay(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
}

void CEnvTrigger_Generic::OnTriggerExit(CCollider* pOther)
{
#ifdef _DEBUG
	char szBuf[128];
	sprintf_s(szBuf, "[EnvTrigger_Generic] Exit <- group %u\n", pOther->Get_RegisteredGroup());
	OutputDebugStringA(szBuf);
#else
	UNREFERENCED_PARAMETER(pOther);
#endif
}

CEnvTrigger_Generic* CEnvTrigger_Generic::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvTrigger_Generic* pInstance = new CEnvTrigger_Generic(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvTrigger_Generic");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvTrigger_Generic::Clone(void* pArg)
{
	CEnvTrigger_Generic* pInstance = new CEnvTrigger_Generic(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvTrigger_Generic");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvTrigger_Generic::Free()
{
	__super::Free();
}

NS_END