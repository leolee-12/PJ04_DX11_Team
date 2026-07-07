#include "Cappy_State_Idle.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CCappy_State_Idle::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CCappy_State_Idle::Get_StateType()
{
	return MONSTER_STATE_TYPE::IDLE;
}

void CCappy_State_Idle::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	Play_IdleAnimation();
}

void CCappy_State_Idle::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr || m_pAnimator == nullptr)
		return;

	_int iType = m_pOwner->Get_AIType();

	if (iType == 1)
	{
		if (m_pAnimator->Get_CurrentAnimName() == "HidingJumpB" && m_pAnimator->Is_Finished())
			Play_IdleAnimation();
	}

}

void CCappy_State_Idle::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CCappy_State_Idle* CCappy_State_Idle::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CCappy_State_Idle* pInstance = new CCappy_State_Idle();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CCappy_State_Idle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy_State_Idle::Play_IdleAnimation()
{
	if (m_pOwner == nullptr || m_pAnimator == nullptr)
		return;

	ANI_PLAY_INFO		Info{};

	_int				iType = m_pOwner->Get_AIType();

	switch (iType)
	{
	case 1:		// 추격형
	{
		Info.strAniName = "HidingWaitA";
		Info.bLoop = false;
		Info.fSpeed = 1.25f;

		m_pAnimator->Play(&Info);

		Info.strAniName = "HidingJumpA";
		Info.bLoop = false;
		Info.fSpeed = 1.25f;

		m_pAnimator->Enqueue(Info);

		Info.strAniName = "HidingWaitB";
		Info.bLoop = false;
		Info.fSpeed = 1.25f;

		m_pAnimator->Enqueue(Info);

		Info.strAniName = "HidingJumpB";
		Info.bLoop = false;
		Info.fSpeed = 1.25f;

		m_pAnimator->Enqueue(Info);

		break;
	}
	case 0:		// 고정형 
	default:	// 0과 동일
	{
		Info.strAniName = "Wait";
		Info.bLoop = true;
		Info.fSpeed = 1.25f;

		m_pAnimator->Play(&Info);
		break;
	}
	}
}

void CCappy_State_Idle::Free()
{
	__super::Free();
}
