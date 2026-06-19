#include "BladeKnight_State_Landing.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"

HRESULT CBladeKnight_State_Landing::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_Landing::Get_StateType()
{
	return MONSTER_STATE_TYPE::LANDING;
}

void CBladeKnight_State_Landing::On_Enter(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

	if (CBladeKnight_Body* pBody = pBladeKnight->Get_Body())
		if (CAnimator* pAnim = pBody->Get_Animator())
			pAnim->Play("Landing", false);
}

void CBladeKnight_State_Landing::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{
	if (nullptr == pBladeKnight)
		return;

	CAnimator* pAnim = pBladeKnight->Get_Body() ? pBladeKnight->Get_Body()->Get_Animator() : nullptr;
	if (pAnim != nullptr && pAnim->Is_Finished())
		pBladeKnight->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CBladeKnight_State_Landing::On_Exit(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

}

CBladeKnight_State_Landing* CBladeKnight_State_Landing::Create()
{
	CBladeKnight_State_Landing* pInstance = new CBladeKnight_State_Landing();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_Landing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_Landing::Free()
{
	__super::Free();
}
