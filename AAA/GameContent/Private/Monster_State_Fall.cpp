#include "Monster_State_Fall.h"
#include "Monster.h"

CMonster_State_Fall::CMonster_State_Fall()
{
}

HRESULT	CMonster_State_Fall::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Fall::Get_StateType()
{
	return MONSTER_STATE_TYPE::FALL;
}

void CMonster_State_Fall::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Get_BlackBoard().bActionFinished = false;
	pMonster->Get_BlackBoard().bCanTransition = true;
	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::FALL);
}

void CMonster_State_Fall::Update(CMonster* pMonster, _float fTimeDelta)
{
}

void CMonster_State_Fall::Exit(CMonster* pMonster)
{
}

CMonster_State_Fall* CMonster_State_Fall::Create()
{
	CMonster_State_Fall* pInstance = new CMonster_State_Fall();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMonster_State_Fall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Fall::Free()
{
	__super::Free();
}
