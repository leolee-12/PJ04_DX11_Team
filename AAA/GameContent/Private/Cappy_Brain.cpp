#include "Cappy_Brain.h"
#include "Monster.h"
#include "Cappy.h"


HRESULT CCappy_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	if (m_pOwner)
		m_iAIType = m_pOwner->Get_AIType();

	return S_OK;
}

void CCappy_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (!Can_Decide(BlackBoard))
		return;

	const MONSTER_STATE_TYPE eCur = m_pOwner->Get_StateType();

	if (eCur == MONSTER_STATE_TYPE::IDLE)
		m_fMaxTime = (m_iAIType == 0) ? s_fIdleWait_Fix : s_fIdleWait_Chase;
	else if (eCur == MONSTER_STATE_TYPE::CHASE)
		m_fMaxTime = s_fChaseTime;

	if (BlackBoard.pTarget == nullptr)
	{
		m_fTimer = 0.f;
		if (eCur != MONSTER_STATE_TYPE::IDLE &&	m_pOwner->Has_State(MONSTER_STATE_TYPE::IDLE))
			m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
		return;
	}

	// IDLE 대기 → DETECT
	if (eCur == MONSTER_STATE_TYPE::IDLE)
	{
		m_fTimer += fTimeDelta;
		if (m_fTimer >= m_fMaxTime)
		{
			m_fTimer = 0.f;                 // 다음 상태 위해 리셋
			m_pOwner->Change_State(MONSTER_STATE_TYPE::DETECT);
		}
		return;
	}

	if (eCur == MONSTER_STATE_TYPE::CHASE)
	{
		m_fTimer += fTimeDelta;
		if (m_fTimer >= m_fMaxTime)
		{
			m_fTimer = 0.f;
			m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
		}
	}
}

CCappy_Brain* CCappy_Brain::Create(CMonster* pOwner)
{
	CCappy_Brain* pInstance = new CCappy_Brain();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CCappy_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy_Brain::Free()
{
	__super::Free();
}
