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

void CMonster_State_KnockBack::On_Enter(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	// 재 진입 시 피격마다 리셋되게 
	m_fTimer = 0.f;

	if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
		pAnim->Play(&m_PlayInfo);
}

void CMonster_State_KnockBack::On_Update(CMonster* pMonster, _float fTimeDelta)
{
	if (pMonster == nullptr)
		return;

	m_fTimer += fTimeDelta;

	CMonster_Movement* pMove = pMonster->Get_Movement();

	const _bool bLanded = (pMove != nullptr && !pMove->Is_Launched());
	const _bool bTimeOut = (m_fTimer >= m_fMaxTime);
	if (!bLanded && !bTimeOut)
		return;					// 런치 + 모든 바운스 동안 루프 유지

	// TODO : 사망 연결 후 IS_Dead() ? Set_Deadt() : 아래 로직
	pMonster->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CMonster_State_KnockBack::On_Exit(CMonster* pMonster)
{
	if (pMonster == nullptr)
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
