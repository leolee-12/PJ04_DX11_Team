#include "BladeKnight_State_Find.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"

HRESULT CBladeKnight_State_Find::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_Find::Get_StateType()
{
	return MONSTER_STATE_TYPE::FIND;
}

void CBladeKnight_State_Find::On_Enter(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

	if (CBladeKnight_Body* pBody = pBladeKnight->Get_Body())
		if (CAnimator* pAnim = pBody->Get_Animator())
		{
			CAnimator::ANI_PLAY_INFO AniInfo{};
			AniInfo.strAniName = "Find";
			AniInfo.bLoop = false;
			AniInfo.fSpeed = 1.25f;
			pAnim->Play(&AniInfo);
		}
}

void CBladeKnight_State_Find::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{
	if (nullptr == pBladeKnight)
		return;

	CAnimator* pAnim = pBladeKnight->Get_Body() ? pBladeKnight->Get_Body()->Get_Animator() : nullptr;
	if (pAnim != nullptr && pAnim->Is_Finished())
		pBladeKnight->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CBladeKnight_State_Find::On_Exit(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

}

CBladeKnight_State_Find* CBladeKnight_State_Find::Create()
{
	CBladeKnight_State_Find* pInstance = new CBladeKnight_State_Find();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_Find");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_Find::Free()
{
	__super::Free();
}
