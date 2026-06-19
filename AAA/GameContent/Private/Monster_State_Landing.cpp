#include "Monster_State_Landing.h"
#include "Monster.h"

CMonster_State_Landing::CMonster_State_Landing()
{
}

HRESULT	CMonster_State_Landing::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Landing::Get_StateType()
{
	return MONSTER_STATE_TYPE::LANDING;
}

void CMonster_State_Landing::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Get_BlackBoard().bActionFinished = false;
	pMonster->Get_BlackBoard().bCanTransition = true;
	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::LANDING);
}

void CMonster_State_Landing::Update(CMonster* pMonster, _float fTimeDelta)
{
}

void CMonster_State_Landing::Exit(CMonster* pMonster)
{
}

CMonster_State_Landing* CMonster_State_Landing::Create()
{
	CMonster_State_Landing* pInstance = new CMonster_State_Landing();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMonster_State_Landing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Landing::Free()
{
	__super::Free();
}
