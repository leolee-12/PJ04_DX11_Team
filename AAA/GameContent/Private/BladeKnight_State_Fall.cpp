#include "BladeKnight_State_Fall.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Monster_Movement.h"

HRESULT CBladeKnight_State_Fall::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_Fall::Get_StateType()
{
	return MONSTER_STATE_TYPE::FALL;
}

void CBladeKnight_State_Fall::On_Enter(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

	if (CBladeKnight_Body* pBody = pBladeKnight->Get_Body())
		if (CAnimator* pAnim = pBody->Get_Animator())
		{
			CAnimator::ANI_PLAY_INFO AniInfo{};
			AniInfo.strAniName = "Fall";
			AniInfo.bLoop = true;
			AniInfo.fSpeed = 1.5f;
			pAnim->Play(&AniInfo);
		}
}

void CBladeKnight_State_Fall::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{
	if (nullptr == pBladeKnight)
		return;
	
	CMonster_Movement* pMove = pBladeKnight->Get_Movement();

	// TODO (넉백 구현 시) 바운스 도중 첫 접촉에서 LANDING으로 빠지지 않도록
	// 마지막 낙하 정착 시점에만 FALL -> LANDING

	if (pMove != nullptr && pMove->Is_Grounded())
		pBladeKnight->Change_State(MONSTER_STATE_TYPE::LANDING);
}

void CBladeKnight_State_Fall::On_Exit(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;
}

CBladeKnight_State_Fall* CBladeKnight_State_Fall::Create()
{
	CBladeKnight_State_Fall* pInstance = new CBladeKnight_State_Fall();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_Fall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_Fall::Free()
{
	__super::Free();
}
