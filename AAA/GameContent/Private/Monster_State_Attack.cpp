#include "Monster_State_Attack.h"
#include "Monster.h"

CMonster_State_Attack::CMonster_State_Attack()
{
}

HRESULT	CMonster_State_Attack::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Attack::Get_StateType()
{
	return MONSTER_STATE_TYPE::ATTACK;
}

void CMonster_State_Attack::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::ATTACK);
}

void CMonster_State_Attack::Update(CMonster* pMonster, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == pMonster)
		return;
}

void CMonster_State_Attack::Exit(CMonster* pMonster)
{
	UNREFERENCED_PARAMETER(pMonster);
}

CMonster_State_Attack* CMonster_State_Attack::Create()
{
	CMonster_State_Attack* pInstance = new  CMonster_State_Attack();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created: CMonster_State_Attack");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Attack::Free()
{
	__super::Free();
}
