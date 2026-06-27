#include "PoppyBrosJr_Brain.h"
#include "PoppyBrosJr.h"

HRESULT CPoppyBrosJr_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CPoppyBrosJr_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	if (!Can_Decide(BlackBoard))
		return;

	MONSTER_STATE_TYPE eCurState = m_pOwner->Get_StateType();
	if (nullptr == BlackBoard.pTarget)
	{
		m_bSpotted = false;
		if (eCurState != MONSTER_STATE_TYPE::IDLE && m_pOwner->Has_State(MONSTER_STATE_TYPE::IDLE))
			m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
		return;
	}

	Decide_Internal(BlackBoard, fTimeDelta);
}

void CPoppyBrosJr_Brain::Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (m_fAttackTimer > 0.f)
		m_fAttackTimer -= fTimeDelta;

	CPoppyBrosJr* pPBJ = static_cast<CPoppyBrosJr*>(m_pOwner);

	const _int iAIType = pPBJ->Get_AIType();	
	const MONSTER_STATE_TYPE eCurState = m_pOwner->Get_StateType();

	const _float fAbsHeight = fabsf(BlackBoard.fHeightToTarget);
	const _bool bAttackable = (iAIType == 0) ? true : (BlackBoard.fDistToTargetXZ <= 6.f && fAbsHeight < 3.f);


	if (bAttackable && m_fAttackTimer <= 0.f)
	{
		m_pOwner->Change_State(MONSTER_STATE_TYPE::ATTACK);
		m_fAttackTimer = s_fAttackCoolTime;
		return;
	}


	// 사거리 밖 추격 안함
	if (eCurState != MONSTER_STATE_TYPE::IDLE)
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

CPoppyBrosJr_Brain* CPoppyBrosJr_Brain::Create(CMonster* pOwner)
{
	CPoppyBrosJr_Brain* pInstance = new CPoppyBrosJr_Brain();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CPoppyBrosJr_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPoppyBrosJr_Brain::Free()
{
	__super::Free();
}
