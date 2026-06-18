#include "BladeKnight_FSM.h"
#include "Monster.h"
#include "BladeKnight.h"

CBladeKnight_FSM::CBladeKnight_FSM()
{
}

HRESULT CBladeKnight_FSM::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	return S_OK;
}

void CBladeKnight_FSM::Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	if (nullptr == pMonster)
		return;

	if (!Can_Decide(pMonster, BlackBoard))
		return;

	// Decide에서 필요한 변수들 지역 변수로 관리
	const _bool bHasTarget = BlackBoard.pTarget != nullptr;
	const _float fAbsHeight = fabsf(BlackBoard.fHeightToTarget);

	// 공격 할 수 있는지 : 타겟 있고, XZ 평면 상의 거리, 높이 차이
	const _bool bTargetAttackable =	bHasTarget && BlackBoard.fDistToTargetXZ <= 2.f && fAbsHeight <= 0.6f;

	// 추격할 수 있는지 : 타겟 있고, 높이 차이, 
	const _bool bTargetChaseable =	bHasTarget && fAbsHeight <= 1.5f;

	CBladeKnight* pBladeKnight = dynamic_cast<CBladeKnight*>(pMonster);

	if (nullptr == pBladeKnight)
	{
		__super::Decide(pMonster, BlackBoard, fTimeDelta);
		return;
	}

	MONSTER_STATE_TYPE  eCurState = pMonster->Get_StateType();

	if (eCurState == MONSTER_STATE_TYPE::ATTACK)
	{
		if (BlackBoard.bActionFinished)
			pMonster->Change_State(pBladeKnight->Get_AttackNewState());

		return;
	}

	// 타겟은 있는데 추격이 불가능할 경우 (RETREAT 이후 IDLE로의 전환)
	if (bHasTarget && !bTargetChaseable)
	{
		if (eCurState != MONSTER_STATE_TYPE::IDLE)
			pMonster->Change_State(MONSTER_STATE_TYPE::IDLE);

		return;
	}

	if (bHasTarget)
	{
		if (bTargetAttackable && pBladeKnight->Try_SelectAttackPattern(BlackBoard.fDistToTargetXZ))
		{
			pMonster->Change_State(MONSTER_STATE_TYPE::ATTACK);
			return;
		}
		
		if (bTargetChaseable && pBladeKnight->Get_AIStyle() == CBladeKnight::BLADEKNIGHT_AI_STYLE::CHASE)
		{
			if (eCurState != MONSTER_STATE_TYPE::CHASE)
				pMonster->Change_State(MONSTER_STATE_TYPE::CHASE);

			return;
		}
	}

	__super::Decide(pMonster, BlackBoard, fTimeDelta);
}

CBladeKnight_FSM* CBladeKnight_FSM::Create()
{
	CBladeKnight_FSM* pInstance = new CBladeKnight_FSM();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBladeKnight_FSM");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_FSM::Free()
{
	__super::Free();
}
