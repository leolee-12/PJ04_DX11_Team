#include "Monster_State_Retreat.h"
#include "Monster.h"
#include "Monster_Movement.h"


CMonster_State_Retreat::CMonster_State_Retreat()
{
}

HRESULT	CMonster_State_Retreat::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Retreat::Get_StateType()
{
	return MONSTER_STATE_TYPE::RETREAT;
}

void CMonster_State_Retreat::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Get_BlackBoard().bActionFinished = false;
	pMonster->Get_BlackBoard().bCanTransition = false;

	if (CMonster_Movement* pMove = pMonster->Get_Movement())
		pMove->Set_LockFacing(true);

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::RETREAT);
}

void CMonster_State_Retreat::Update(CMonster* pMonster, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == pMonster)
		return;

	if (pMonster->Is_StateAnimationFinished())
	{
		pMonster->Get_BlackBoard().bActionFinished = true;
		pMonster->Get_BlackBoard().bCanTransition = true;
		return;
	}

	_vector vLook = pMonster->Get_Transform()->Get_State(STATE::LOOK);
	_float3 vBack = {};
	XMStoreFloat3(&vBack, XMVectorNegate(XMVectorSetY(vLook, 0.f)));

	if (!pMonster->Get_BlackBoard().bMoveLocked)
		pMonster->Add_MoveDir(vBack);
}

void CMonster_State_Retreat::Exit(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	if (CMonster_Movement* pMove = pMonster->Get_Movement())
		pMove->Set_LockFacing(false);

	pMonster->Get_BlackBoard().bMoveLocked = false;
}

CMonster_State_Retreat* CMonster_State_Retreat::Create()
{
	CMonster_State_Retreat* pInstance = new CMonster_State_Retreat();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMonster_State_Retreat");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Retreat::Free()
{
	__super::Free();
}
