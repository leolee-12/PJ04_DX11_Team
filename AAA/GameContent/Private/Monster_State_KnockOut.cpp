#include "Monster_State_KnockOut.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_KnockOut::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_fMaxTime = 1.0f;

	return S_OK;
}

void CMonster_State_KnockOut::Enter(MONSTER_STATE_TYPE ePrevState)
{
	__super::Enter(ePrevState);

	if (nullptr == m_pOwner)
		return;

	if (m_pMovement && m_pMovement->Is_Launched())
		m_pOwner->Start_LaunchSmokeFx();
	else
		m_pOwner->Stop_LaunchSmokeFx();
}

void CMonster_State_KnockOut::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pMovement && m_pMovement->Is_Launched())
		m_pOwner->Start_LaunchSmokeFx();
	else
		m_pOwner->Stop_LaunchSmokeFx();

	__super::Update(fTimeDelta);

	if (m_pAnimator)
		m_pAnimator->SpinByProgress("RotL", 1.f, XMVectorSet(1.f, 0.f, 0.f, 0.f));
}

void CMonster_State_KnockOut::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner)
		m_pOwner->Stop_LaunchSmokeFx();

	__super::Exit(eNextState);
}

MONSTER_STATE_TYPE CMonster_State_KnockOut::Get_StateType()
{
	return MONSTER_STATE_TYPE::KNOCK_OUT;
}

void CMonster_State_KnockOut::Apply_DeathLaunch(_fvector vAttackerPos, _float fStrength)
{
	if (m_pMovement)
		m_pMovement->KO(vAttackerPos, fStrength);
}

CMonster_State_KnockOut* CMonster_State_KnockOut::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_KnockOut* pInstance = new CMonster_State_KnockOut();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_KnockOut");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_KnockOut::Free()
{
	__super::Free();
}
