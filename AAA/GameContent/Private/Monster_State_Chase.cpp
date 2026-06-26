#include "Monster_State_Chase.h"
#include "Monster.h"
#include "Monster_Movement.h"

MONSTER_STATE_TYPE CMonster_State_Chase::Get_StateType()
{
	return MONSTER_STATE_TYPE::CHASE;
}

void CMonster_State_Chase::Apply_Movement(_float fTimeDelta)
{
	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();

	if (BB.pTarget == nullptr)
		return;

	m_pOwner->Add_MoveDir(BB.vDirToTargetXZ);
}

CMonster_State_Chase* CMonster_State_Chase::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Chase* pInstance = new CMonster_State_Chase();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Chase");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Chase::Free()
{
	__super::Free();
}
