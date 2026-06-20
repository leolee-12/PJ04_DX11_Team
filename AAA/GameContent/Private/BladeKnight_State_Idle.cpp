#include "BladeKnight_State_Idle.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"

HRESULT CBladeKnight_State_Idle::Initialize()
{ 
	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_Idle::Get_StateType()
{
	return MONSTER_STATE_TYPE::IDLE;
}

void CBladeKnight_State_Idle::On_Enter(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;
	
	if (CBladeKnight_Body* pBody = pBladeKnight->Get_Body())
		if ( CAnimator* pAnim = pBody->Get_Animator())
			pAnim->Play("FindWait", true);
}

void CBladeKnight_State_Idle::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{

}

void CBladeKnight_State_Idle::On_Exit(CBladeKnight* pBladeKnight)
{

}

CBladeKnight_State_Idle* CBladeKnight_State_Idle::Create()
{
	CBladeKnight_State_Idle* pInstance = new CBladeKnight_State_Idle();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_Idle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_Idle::Free()
{
	__super::Free();
}
