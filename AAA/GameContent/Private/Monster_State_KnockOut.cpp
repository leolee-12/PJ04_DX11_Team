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

void CMonster_State_KnockOut::On_Enter(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	m_fTimer = 0.f;

	if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
		pAnim->Play(&m_PlayInfo);
}

void CMonster_State_KnockOut::On_Update(CMonster* pMonster, _float fTimeDelta)
{
	if (pMonster == nullptr)
		return;

	m_fTimer += fTimeDelta;

	CMonster_Movement* pMove = pMonster->Get_Movement();

	const _bool bLanded = (pMove != nullptr && !pMove->Is_Launched());
	const _bool bTimeOut = (m_fTimer >= m_fMaxTime);
	if (!bLanded && !bTimeOut)
		return;					// 런치 + 모든 바운스 동안 루프 유지

	pMonster->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CMonster_State_KnockOut::On_Exit(CMonster* pMonster)
{
	if (pMonster == nullptr)
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
