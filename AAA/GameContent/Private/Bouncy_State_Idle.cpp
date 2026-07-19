#include "Bouncy_State_Idle.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CBouncy_State_Idle::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;
	return S_OK;
}

MONSTER_STATE_TYPE CBouncy_State_Idle::Get_StateType()
{
	return MONSTER_STATE_TYPE::IDLE;
}

void CBouncy_State_Idle::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pAnimator || nullptr == m_pMovement)
		return;

	m_fRestTimer = s_fRestTime;
	m_bSequenceStarted = false;
	m_bJumped = false;

	if (m_fRestTimer <= 0.f)
		Begin_Sequence();
}

void CBouncy_State_Idle::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner || nullptr == m_pAnimator || nullptr == m_pMovement)
		return;

	// rest timer -> start jump sequence
	if (!m_bSequenceStarted)
	{
		m_fRestTimer -= fTimeDelta;
		if (m_fRestTimer <= 0.f)
			Begin_Sequence();
		return;
	}

	// impulse on the first frame "Jump" becomes current clip
	// (queued here, consumed by Move() while grounded)
	if (!m_bJumped && m_pAnimator->Get_CurrentAnimName() == "JumpS")
	{
		m_pMovement->Jump();
		m_bJumped = true;
	}

	// safety net: impulse never left the ground -> re-hop
	if (m_bJumped && m_pMovement->Is_Grounded() && m_pAnimator->Is_Finished())
		Begin_Sequence();

	// descent -> FALL is delegated to common Check_AirborneReflex
}

void CBouncy_State_Idle::Exit(MONSTER_STATE_TYPE eNextState)
{
}

void CBouncy_State_Idle::Begin_Sequence()
{
	if (nullptr == m_pAnimator)
		return;

	m_fRestTimer = 0.f;
	m_bSequenceStarted = true;
	m_bJumped = false;

	ANI_PLAY_INFO Info{};
	Info.strAniName = "JumpStart";
	Info.bLoop = false;
	Info.fBlend = s_fJumpStartBlend;
	Info.fSpeed = 1.f;
	m_pAnimator->Play(&Info);

	Info.strAniName = "JumpS";
	Info.bLoop = false;
	Info.fBlend = s_fJumpSBlend;
	Info.fSpeed = 1.f;
	m_pAnimator->Enqueue(Info);

	Info.strAniName = "Jump";
	Info.bLoop = false;
	Info.fBlend = s_fJumpBlend;
	Info.fSpeed = 1.f;
	m_pAnimator->Enqueue(Info);
}

CBouncy_State_Idle* CBouncy_State_Idle::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBouncy_State_Idle* pInstance = new CBouncy_State_Idle();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBouncy_State_Idle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBouncy_State_Idle::Free()
{
	__super::Free();
}