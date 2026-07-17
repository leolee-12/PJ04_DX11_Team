#include "Dekabu_Brain.h"
#include "Dekabu.h"

CDekabu_Brain::CDekabu_Brain()
{
}

HRESULT CDekabu_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CDekabu_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == m_pOwner)
		return;

	if (!Can_Decide(BlackBoard))
		return;

	if (m_pOwner->Get_StateType() != MONSTER_STATE_TYPE::IDLE)
		return;

	if (nullptr == BlackBoard.pTarget)
		return;

	if (m_pOwner->Has_State(MONSTER_STATE_TYPE::WINDUP))
		m_pOwner->Change_State(MONSTER_STATE_TYPE::WINDUP);
}

CDekabu_Brain* CDekabu_Brain::Create(CMonster* pOwner)
{
	CDekabu_Brain* pInstance = new CDekabu_Brain();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CDekabu_Brain");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDekabu_Brain::Free()
{
	__super::Free();
}
