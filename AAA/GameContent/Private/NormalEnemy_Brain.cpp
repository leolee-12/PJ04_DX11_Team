#include "NormalEnemy_Brain.h"
#include "Monster.h"
#include "NormalEnemy.h"

CNormalEnemy_Brain::CNormalEnemy_Brain()
{
}

HRESULT CNormalEnemy_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	m_fMaxTime = TRANSITION_TIME;

	return S_OK;
}

void CNormalEnemy_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	// 능동 사이클 
	if (!Can_Decide(BlackBoard))
		return;

	const MONSTER_STATE_TYPE eCur = m_pOwner->Get_StateType();

	// 1) 감지 -> DETECT ( 매 발견마다 )
	if (nullptr != BlackBoard.pTarget)
	{
		if (eCur != MONSTER_STATE_TYPE::DETECT)
			m_pOwner->Change_State(MONSTER_STATE_TYPE::DETECT);
		return;
	}

	// 2) 평상시 IDLE 에서 일정시간마다 FIND 상태 전이
	if (eCur == MONSTER_STATE_TYPE::IDLE)
	{
		m_fMaxTime -= fTimeDelta;
		if (m_fMaxTime <= 0.f)
		{
			m_fMaxTime = TRANSITION_TIME;

			MONSTER_STATE_TYPE eNext = MONSTER_STATE_TYPE::FIND;

			const _int iAIType = m_pOwner->Get_AIType();
			const _bool bHasPatrol = m_pOwner->Has_State(MONSTER_STATE_TYPE::PATROL);

			// 자유형(1) : FIND  / PATROL 랜덤 선택
			// 고정형 (0) : FIND 
			if (iAIType == 1 && bHasPatrol && (rand() % 2 == 0))
				eNext = MONSTER_STATE_TYPE::PATROL;

			m_pOwner->Change_State(eNext);
		}
	}
}

CNormalEnemy_Brain* CNormalEnemy_Brain::Create(CMonster* pOwner)
{
	CNormalEnemy_Brain* pInstance = new CNormalEnemy_Brain();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CNormalEnemy_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CNormalEnemy_Brain::Free()
{
	__super::Free();
}
