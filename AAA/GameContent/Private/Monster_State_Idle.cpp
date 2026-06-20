#include "Monster_State_Idle.h"
#include "Monster.h"

HRESULT CMonster_State_Idle::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Idle::Get_StateType()
{
	return MONSTER_STATE_TYPE::IDLE;
}

void CMonster_State_Idle::On_Enter(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
		pAnim->Play(&m_PlayInfo);
}

void CMonster_State_Idle::On_Update(CMonster* pMonster, _float fTimeDelta)
{
	if (pMonster == nullptr)
		return;
}

void CMonster_State_Idle::On_Exit(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;
}

CMonster_State_Idle* CMonster_State_Idle::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Idle* pInstance = new CMonster_State_Idle();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Idle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Idle::Free()
{
	__super::Free();
}
