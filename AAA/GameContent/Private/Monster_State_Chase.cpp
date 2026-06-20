#include "Monster_State_Chase.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Chase::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Chase::Get_StateType()
{
	return MONSTER_STATE_TYPE::CHASE;
}

void CMonster_State_Chase::On_Enter(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	if (CMonster_Movement* pMove = pMonster->Get_Movement())
		pMove->Set_MoveSpeed(m_fSpeed);

	if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
		pAnim->Play(&m_PlayInfo);
}

void CMonster_State_Chase::On_Update(CMonster* pMonster, _float fTimeDelta)
{
	if (pMonster == nullptr)
		return;

	const MONSTER_BLACKBOARD& BB = pMonster->Get_BlackBoard();

	if (BB.pTarget == nullptr)
		return;

	// TODO : 지평면에서만 움직이는 애들 / 공중에서 추격하는 애들 구분할 것
	pMonster->Add_MoveDir(BB.vDirToTargetXZ);
}

void CMonster_State_Chase::On_Exit(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;
}

CMonster_State_Chase* CMonster_State_Chase::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Chase* pInstance = new CMonster_State_Chase();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Chase");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Chase::Free()
{
	__super::Free();
}
