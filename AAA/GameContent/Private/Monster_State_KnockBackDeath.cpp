#include "Monster_State_KnockBackDeath.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_KnockBackDeath::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_fMaxTime = 0.5f;

	return S_OK;
}

void CMonster_State_KnockBackDeath::Enter(MONSTER_STATE_TYPE ePrevState)
{
	__super::Enter(ePrevState);

	if (nullptr == m_pOwner)
		return;

	if (m_pMovement && m_pMovement->Is_Launched())
		m_pOwner->Start_LaunchSmokeFx();
	else
		m_pOwner->Stop_LaunchSmokeFx();
}

MONSTER_STATE_TYPE CMonster_State_KnockBackDeath::Get_StateType()
{
	return MONSTER_STATE_TYPE::KNOCK_BACK_DEATH;
}

void CMonster_State_KnockBackDeath::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pMovement && m_pMovement->Is_Launched())
		m_pOwner->Start_LaunchSmokeFx();
	else
		m_pOwner->Stop_LaunchSmokeFx();

	__super::Update(fTimeDelta);

	if (m_pAnimator)
		m_pAnimator->TiltByProgress("RotL", 30.f, XMVectorSet(1.f, 0.f, 0.f, 0.f));
}

void CMonster_State_KnockBackDeath::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner)
		m_pOwner->Stop_LaunchSmokeFx();

	__super::Exit(eNextState);
}

void CMonster_State_KnockBackDeath::Apply_DeathLaunch(_fvector vAttackerPos, _float fStrength)
{
	if (m_pMovement)
		m_pMovement->Knockback(vAttackerPos, fStrength);
}

CMonster_State_KnockBackDeath* CMonster_State_KnockBackDeath::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_KnockBackDeath* pInstance = new CMonster_State_KnockBackDeath();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_KnockBackDeath");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_KnockBackDeath::Free()
{
	__super::Free();
}
