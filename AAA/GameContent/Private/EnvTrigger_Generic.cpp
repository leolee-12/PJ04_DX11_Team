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

HRESULT CEnvTrigger_Generic::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvTrigger_Generic::Initialize(void* pArg)
{
	return __super::Initialize(pArg);
}

void CEnvTrigger_Generic::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

void CEnvTrigger_Generic::OnTriggerEnter(CCollider* pOther)
{
	char szBuf[128];
	sprintf_s(szBuf, "[EnvTrigger_Generic] Enter <- group %u\n", pOther->Get_RegisteredGroup());
	OutputDebugStringA(szBuf);
}

void CEnvTrigger_Generic::OnTriggerStay(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
	OutputDebugStringA("[EnvTrigger_Generic] Stay\n");
}

void CEnvTrigger_Generic::OnTriggerExit(CCollider* pOther)
{
	char szBuf[128];
	sprintf_s(szBuf, "[EnvTrigger_Generic] Exit  <- group %u\n", pOther->Get_RegisteredGroup());
	OutputDebugStringA(szBuf);
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