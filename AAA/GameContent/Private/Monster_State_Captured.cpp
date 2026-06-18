#include "Monster_State_Captured.h"
#include "Monster.h"

CMonster_State_Captured::CMonster_State_Captured()
{
}

HRESULT	CMonster_State_Captured::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Captured::Get_StateType()
{
	return MONSTER_STATE_TYPE::CAPTURED;
}

void CMonster_State_Captured::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Get_BlackBoard().bActionFinished = false;
	pMonster->Get_BlackBoard().bCanTransition = false;

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::CAPTURED);
}

void CMonster_State_Captured::Update(CMonster* pMonster, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == pMonster)
		return;
}

void CMonster_State_Captured::Exit(CMonster* pMonster)
{
	UNREFERENCED_PARAMETER(pMonster);
}

CMonster_State_Captured* CMonster_State_Captured::Create()
{
	CMonster_State_Captured* pInstance = new  CMonster_State_Captured();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created: CMonster_State_Captured");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Captured::Free()
{
	__super::Free();
}
