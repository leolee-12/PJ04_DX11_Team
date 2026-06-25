#include "Monster_State_Death.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Death::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = false;
	m_fMaxTime = 2.0f;

	return S_OK;
}

void CMonster_State_Death::Enter()
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);

	if (m_pMovement)
	{
		const CMonster::HIT_REACTION& tHit = m_pOwner->Get_LastHit();
		Apply_DeathLaunch(XMLoadFloat3(&tHit.vAttackerPos), tHit.fKnockBack);
	}
}

void CMonster_State_Death::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_fMaxTime -= fTimeDelta;
	if (m_fMaxTime <= 0.f)
		m_pOwner->Despawn();
}

void CMonster_State_Death::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;
}

void CMonster_State_Death::Free()
{
	__super::Free();
}
