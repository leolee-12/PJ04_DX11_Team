#include "PoppyBrosJr_State_Throw.h"
#include "PoppyBrosJr.h"

HRESULT CPoppyBrosJr_State_Throw::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_eNextState = MONSTER_STATE_TYPE::IDLE;
	return S_OK;
}

MONSTER_STATE_TYPE CPoppyBrosJr_State_Throw::Get_StateType()
{
	return MONSTER_STATE_TYPE::ATTACK;
}

void CPoppyBrosJr_State_Throw::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;


	if (m_pAnimator)
		Play_AttackAnimation();
}

void CPoppyBrosJr_State_Throw::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && m_pAnimator->Is_Finished())
	{
		const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
		m_pOwner->Change_State((nullptr != BB.pTarget)
			? MONSTER_STATE_TYPE::WINDUP    // 타겟 유지 → 재차지
			: MONSTER_STATE_TYPE::IDLE);    // 상실 → 대기
		return;
	}
}

void CPoppyBrosJr_State_Throw::Exit(MONSTER_STATE_TYPE eNextState)
{
	__super::Exit(eNextState);

	if (m_pOwner)
		static_cast<CPoppyBrosJr*>(m_pOwner)->Release_Bomb();
}

void CPoppyBrosJr_State_Throw::Play_AttackAnimation()
{
	if (m_pAnimator)
	{
		ANI_PLAY_INFO Info{};
		Info.strAniName = "Throw";
		Info.bLoop = false;
		Info.fSpeed = 1.25f;
		m_pAnimator->Play(&Info);
	}
}

CPoppyBrosJr_State_Throw* CPoppyBrosJr_State_Throw::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CPoppyBrosJr_State_Throw* pInstance = new CPoppyBrosJr_State_Throw();

	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CPoppyBrosJr_State_Throw");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPoppyBrosJr_State_Throw::Free()
{
	__super::Free();
}
