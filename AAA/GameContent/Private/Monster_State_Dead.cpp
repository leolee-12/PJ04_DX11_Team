#include "Monster_State_Dead.h"
#include "Monster.h"

CMonster_State_Dead::CMonster_State_Dead()
{
}

HRESULT	CMonster_State_Dead::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Dead::Get_StateType()
{
	return MONSTER_STATE_TYPE::DEAD;
}

void CMonster_State_Dead::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Get_BlackBoard().bActionFinished = false;
	pMonster->Get_BlackBoard().bCanTransition = false;

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::DEAD);
}

void CMonster_State_Dead::Update(CMonster* pMonster, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
}

void CMonster_State_Dead::Exit(CMonster* pMonster)
{
	UNREFERENCED_PARAMETER(pMonster);
}

CMonster_State_Dead* CMonster_State_Dead::Create()
{
	CMonster_State_Dead* pInstance = new  CMonster_State_Dead();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created: CMonster_State_Dead");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Dead::Free()
{
	__super::Free();
}
