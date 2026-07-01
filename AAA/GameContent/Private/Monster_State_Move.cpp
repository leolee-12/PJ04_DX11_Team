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

void CMonster_State_Move::Enter(MONSTER_STATE_TYPE ePrevState)
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

	// 일반 이동 : 고정 속도 그대로
	if (!Is_UseMoveWindow())
	{
		Apply_Movement(fTimeDelta);
		return;
	}

	const auto& bb = m_pOwner->Get_BlackBoard();
	if (!bb.bCanMove)
		return;

	_float lo = bb.fMoveWinLo;
	_float span = bb.fMoveWinHi - lo;
	_float p = 0.5f;

	if (span > 1e-4f && m_pAnimator)
		p = (m_pAnimator->Get_Progress() - lo) / span;

	if (m_pMovement)
		m_pMovement->Set_WindowMoveSpeed(m_fSpeed, p);

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
