#include "Monster_State.h"
#include "Monster.h"
#include "Monster_Movement.h"

CMonster_State::CMonster_State()
	: m_bIsInterruptible{ false }
{
}

MONSTER_STATE_TYPE CMonster_State::Get_NextState()
{
	return MONSTER_STATE_TYPE::IDLE;
}

HRESULT	CMonster_State::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	m_PlayInfo = tInfo;
	m_fSpeed = fSpeed;
	m_bIsInterruptible = false;

	return S_OK;
}

void CMonster_State::Set_Owner(CMonster* pOwner)
{
	m_pOwner	= pOwner;
	m_pAnimator = m_pOwner->Get_BodyAnimator();
	m_pMovement = m_pOwner->Get_Movement();
}

void CMonster_State::Free()
{
	__super::Free();
}
