#include "BladeKnight_Brain.h"
#include "Monster.h"
#include "BladeKnight.h"

CBladeKnight_Brain::CBladeKnight_Brain()
{
}

HRESULT CBladeKnight_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CBladeKnight_Brain::Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == m_pOwner)
		return;

	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(m_pOwner);

	const _float fAbsHeight = fabsf(BlackBoard.fHeightToTarget);
	const _bool bAttackable = BlackBoard.fDistToTargetXZ <= 2.f && fAbsHeight < 0.6f;
	const _bool bChaseable = fAbsHeight <= 1.5f;

	const MONSTER_STATE_TYPE eCurState = m_pOwner->Get_StateType();

	if (bAttackable)
	{
		m_pOwner->Change_State(Pick_AttackState(pBladeKnight->Get_AIType()));
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

CBladeKnight_Brain* CBladeKnight_Brain::Create(CMonster* pMonster)
{
	CBladeKnight_Brain* pInstance = new CBladeKnight_Brain();

	if (FAILED(pInstance->Initialize(pMonster)))
	{
		MSG_BOX("Failed to Created : CBladeKnight_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

MONSTER_STATE_TYPE CBladeKnight_Brain::Pick_AttackState(_int iAIType)
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

void CBladeKnight_Brain::Free()
{
	__super::Free();
}
