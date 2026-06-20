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

void CBladeKnight_FSM::On_Decide_Combat(CBladeKnight* pBladeKnight, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == pBladeKnight)
		return;

	const _float fAbsHeight = fabsf(BlackBoard.fHeightToTarget);
	const _bool bAttackable = BlackBoard.fDistToTargetXZ <= 2.f && fAbsHeight < 0.6f;
	const _bool bChaseable = fAbsHeight <= 1.5f;

	const MONSTER_STATE_TYPE eCurState = pBladeKnight->Get_StateType();

	if (bAttackable)
	{
		pBladeKnight->Change_State(Pick_AttackState(pBladeKnight->Get_AIType()));
		return;
	}

	if (bChaseable && pBladeKnight->Get_AIType() == 1)	// 1은 자유 이동형
	{
		if (eCurState != MONSTER_STATE_TYPE::CHASE)
			pBladeKnight->Change_State(MONSTER_STATE_TYPE::CHASE);
		return;
	}
	
	// 사거리 밖 추격 안함
	if (eCurState != MONSTER_STATE_TYPE::IDLE)
		pBladeKnight->Change_State(MONSTER_STATE_TYPE::IDLE);
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

MONSTER_STATE_TYPE CBladeKnight_FSM::Pick_AttackState(_int iAIType)
{
	static const MONSTER_STATE_TYPE ChasePool[] =
	{
		MONSTER_STATE_TYPE::ATTACK, MONSTER_STATE_TYPE::DOUBLE_ATTACK, MONSTER_STATE_TYPE::TORNADO_ATTACK 
	};

	static const MONSTER_STATE_TYPE StationaryPool[] =
	{
		MONSTER_STATE_TYPE::ATTACK, MONSTER_STATE_TYPE::ATTACK, MONSTER_STATE_TYPE::TORNADO_ATTACK 
	};

	const _bool bChase = (iAIType == 1);
	const MONSTER_STATE_TYPE* pPool = bChase ? ChasePool : StationaryPool;
	const _uint iCount = bChase ? _countof(ChasePool) : _countof(StationaryPool);

	if (m_iAttackIndex >= iCount)
		m_iAttackIndex = 0;

	const MONSTER_STATE_TYPE eAttack = pPool[m_iAttackIndex];
	++m_iAttackIndex;

	return eAttack;
}

void CBladeKnight_FSM::Free()
{
	__super::Free();
}
