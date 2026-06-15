#include "Monster_State_Idle.h"
#include "Monster.h"

CMonster_State_Idle::CMonster_State_Idle()
{
}

HRESULT	CMonster_State_Idle::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Idle::Get_StateType()
{
	return MONSTER_STATE_TYPE::IDLE;
}

void CMonster_State_Idle::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::IDLE);
}

void CMonster_State_Idle::Update(CMonster* pMonster, _float fTimeDelta)
{
}

void CMonster_State_Idle::Exit(CMonster* pMonster)
{
}

CMonster_State_Idle* CMonster_State_Idle::Create()
{
	CMonster_State_Idle* pInstance = new CMonster_State_Idle();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMonster_State_Idle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Idle::Free()
{
	__super::Free();
}
