#include "BladeKnight_State_Chase.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Monster_Movement.h"

HRESULT CBladeKnight_State_Chase::Initialize()
{
	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_Chase::Get_StateType()
{
	return MONSTER_STATE_TYPE::CHASE;
}

void CBladeKnight_State_Chase::On_Enter(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

	if (CMonster_Movement* pMovement = pBladeKnight->Get_Movement())
		pMovement->Set_MoveSpeed(3.f);

	if (CBladeKnight_Body* pBody = pBladeKnight->Get_Body())
		if (CAnimator* pAnim = pBody->Get_Animator())
		{
			CAnimator::ANI_PLAY_INFO AniInfo{};
			AniInfo.strAniName = "Move";
			AniInfo.bLoop = true;
			AniInfo.fSpeed = 1.5f;
			pAnim->Play(&AniInfo);
		}
}

void CBladeKnight_State_Chase::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{
	if (nullptr == pBladeKnight)
		return;

	const MONSTER_BLACKBOARD& BB = pBladeKnight->Get_BlackBoard();
	if (BB.pTarget == nullptr)
		return;

	pBladeKnight->Add_MoveDir(BB.vDirToTargetXZ);
}

void CBladeKnight_State_Chase::On_Exit(CBladeKnight* pBladeKnight)
{
}

CBladeKnight_State_Chase* CBladeKnight_State_Chase::Create()
{
	CBladeKnight_State_Chase* pInstance = new CBladeKnight_State_Chase();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_Chase");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_Chase::Free()
{
	__super::Free();
}
