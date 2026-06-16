#include "Monster_State_Retreat.h"
#include "Monster.h"

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

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::RETREAT);
}

void CMonster_State_Retreat::Update(CMonster* pMonster, _float fTimeDelta)
{
}

void CMonster_State_Retreat::Exit(CMonster* pMonster)
{
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
