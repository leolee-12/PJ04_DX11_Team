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


	const _int iAIType = m_pOwner->Get_AIType();

	const _float fAbsHeight = fabsf(BlackBoard.fHeightToTarget);
	const _bool bAttackable = (iAIType == 1)
		? (BlackBoard.fDistToTargetXZ <= 2.5f && fAbsHeight < 3.f)
		: (fAbsHeight < 7.f);
	const _bool bChaseable = (iAIType == 1) ? (fAbsHeight <= 5.f) : false;

	const MONSTER_STATE_TYPE eCurState = m_pOwner->Get_StateType();

	if (bAttackable)
	{
		m_pOwner->Change_State(Pick_AttackState(iAIType));
		return;
	}

	if (bChaseable && iAIType == 1)	// 1은 자유 이동형
	{
		if (eCurState != MONSTER_STATE_TYPE::CHASE)
			m_pOwner->Change_State(MONSTER_STATE_TYPE::CHASE);
		return;
	}
	
	// 사거리 밖 추격 안함
	if (eCurState != MONSTER_STATE_TYPE::IDLE)
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
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
