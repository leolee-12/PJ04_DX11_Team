#include "Monster_State_KnockBack.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_KnockBack::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_fMaxTime = 2.0f;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_KnockBack::Get_StateType()
{
	return MONSTER_STATE_TYPE::KNOCK_BACK;
}

void CMonster_State_KnockBack::Enter()
{
	if (m_pOwner == nullptr)
		return;

	// 재 진입 시 피격마다 리셋되게 
	m_fTimer = 0.f;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);

	if (m_pMovement)
	{
		const CMonster::HIT_REACTION& LastHit = m_pOwner->Get_LastHit();
		m_pMovement->Knockback(XMLoadFloat3(&LastHit.vAttackerPos), LastHit.fKnockBack);
	}
}

void CMonster_State_KnockBack::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer += fTimeDelta;

	if (m_pAnimator == nullptr)
		return;

	if (m_pAnimator)
		m_pAnimator->TiltByProgress("RotL", 30.f, XMVectorSet(1.f, 0.f, 0.f, 0.f));

	const _bool bLanded = (m_pMovement && !m_pMovement->Is_Launched());
	const _bool bTimeOut = (m_fTimer >= m_fMaxTime);
	if (!bLanded && !bTimeOut)
		return;

	m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);

}

void CMonster_State_KnockBack::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CMonster_State_KnockBack* CMonster_State_KnockBack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_KnockBack* pInstance = new CMonster_State_KnockBack();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_KnockBack");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_KnockBack::Free()
{
	__super::Free();
}
