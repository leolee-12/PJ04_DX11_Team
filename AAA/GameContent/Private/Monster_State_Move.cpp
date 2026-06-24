#include "Monster_State_Move.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Move::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

void CMonster_State_Move::Enter()
{
	if (m_pOwner == nullptr)
		return;

	if (m_pMovement)
		m_pMovement->Set_MoveSpeed(m_fSpeed);

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Move::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	// MoveWindow를 쓰는 상태라면  bCanMove여야 한다라는 의미
	if (!Is_UseMoveWindow() || m_pOwner->Get_BlackBoard().bCanMove)
		Apply_Movement(fTimeDelta);
}

void CMonster_State_Move::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

void CMonster_State_Move::Free()
{
	__super::Free();
}
