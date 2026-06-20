#include "BladeKnight_State_Retreat.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Monster_Movement.h"

HRESULT CBladeKnight_State_Retreat::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_Retreat::Get_StateType()
{
	return MONSTER_STATE_TYPE::RETREAT;
}

void CBladeKnight_State_Retreat::On_Enter(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

	if (CMonster_Movement* pMovement = pBladeKnight->Get_Movement())
	{
		pMovement->Set_MoveSpeed(2.f);
		pMovement->Set_LockFacing(true);		// 바라보던 방향 유지하며 뒷걸음질
	}

	if (CBladeKnight_Body* pBody = pBladeKnight->Get_Body())
		if (CAnimator* pAnim = pBody->Get_Animator())
		{
			CAnimator::ANI_PLAY_INFO AniInfo{};
			AniInfo.strAniName = "Retreat";
			AniInfo.bLoop = false;
			AniInfo.fSpeed = 1.5f;
			pAnim->Play(&AniInfo);
		}
}

void CBladeKnight_State_Retreat::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{
	if (nullptr == pBladeKnight)
		return;

	CAnimator* pAnim = pBladeKnight->Get_Body() ? pBladeKnight->Get_Body()->Get_Animator() : nullptr;
	if (pAnim != nullptr && pAnim->Is_Finished())
	{
		pBladeKnight->Change_State(MONSTER_STATE_TYPE::IDLE);		// 후퇴 끝 -> IDLE 
		return;
	}

	if (!pBladeKnight->Get_BlackBoard().bMoveLocked)
	{
		_vector vLook = pBladeKnight->Get_Transform()->Get_State(STATE::LOOK);

		_float3 vBack{};
		XMStoreFloat3(&vBack, XMVectorNegate(XMVectorSetY(vLook, 0.f)));
		pBladeKnight->Add_MoveDir(vBack);
	}
}

void CBladeKnight_State_Retreat::On_Exit(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

	if (CMonster_Movement* pMovement = pBladeKnight->Get_Movement())
		pMovement->Set_LockFacing(false);
}

CBladeKnight_State_Retreat* CBladeKnight_State_Retreat::Create()
{
	CBladeKnight_State_Retreat* pInstance = new CBladeKnight_State_Retreat();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_Retreat");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_Retreat::Free()
{
	__super::Free();
}
