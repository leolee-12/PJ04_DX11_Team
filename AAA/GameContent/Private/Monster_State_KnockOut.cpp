#include "Monster_State_KnockOut.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_KnockOut::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_fMaxTime = 3.f;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_KnockOut::Get_StateType()
{
	return MONSTER_STATE_TYPE::KNOCK_OUT;
}

void CMonster_State_KnockOut::Enter()
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer = 0.f;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_KnockOut::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer += fTimeDelta;

	const _bool bLanded = (m_pMovement && !m_pMovement->Is_Launched());
	const _bool bTimeOut = (m_fTimer >= m_fMaxTime);
	if (!bLanded && !bTimeOut)
		return;					// 런치 + 모든 바운스 동안 루프 유지

	m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CMonster_State_KnockOut::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
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
