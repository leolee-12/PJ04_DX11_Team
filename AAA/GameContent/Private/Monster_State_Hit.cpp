#include "Monster_State_Hit.h"
#include "Monster.h"

CMonster_State_Hit::CMonster_State_Hit()
{
}

HRESULT	CMonster_State_Hit::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Hit::Get_StateType()
{
	return MONSTER_STATE_TYPE::HIT;
}

void CMonster_State_Hit::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Get_BlackBoard().bActionFinished = false;
	pMonster->Get_BlackBoard().bCanTransition = false;

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::HIT);
}

void CMonster_State_Hit::Update(CMonster* pMonster, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == pMonster)
		return;

	if (pMonster->Is_StateAnimationFinished())
	{
		pMonster->Get_BlackBoard().bActionFinished = true;
		pMonster->Get_BlackBoard().bCanTransition = true;
	}

	// 임시 작성
	pMonster->Add_MoveDir(_float3({ 0.f, 0.f, 1.f }));
}

void CMonster_State_Hit::Exit(CMonster* pMonster)
{
	UNREFERENCED_PARAMETER(pMonster);
}

CMonster_State_Hit* CMonster_State_Hit::Create()
{
	CMonster_State_Hit* pInstance = new  CMonster_State_Hit();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created: CMonster_State_Hit");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Hit::Free()
{
	__super::Free();
}
