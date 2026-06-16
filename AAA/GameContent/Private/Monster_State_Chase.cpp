#include "Monster_State_Chase.h"
#include "Monster.h"

CMonster_State_Chase::CMonster_State_Chase()
{
}

HRESULT	CMonster_State_Chase::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Chase::Get_StateType()
{
	return MONSTER_STATE_TYPE::CHASE;
}

void CMonster_State_Chase::Enter(CMonster* pMonster)
{
	if (nullptr == pMonster)
		return;

	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::CHASE);
}

void CMonster_State_Chase::Update(CMonster* pMonster, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == pMonster)
		return;

	// 임시 작성
	pMonster->Add_MoveDir(_float3({ 0.f, 0.f, 1.f }));
}

void CMonster_State_Chase::Exit(CMonster* pMonster)
{
	UNREFERENCED_PARAMETER(pMonster);
}

CMonster_State_Chase* CMonster_State_Chase::Create()
{
	CMonster_State_Chase* pInstance = new  CMonster_State_Chase();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created: CMonster_State_Chase");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Chase::Free()
{
	__super::Free();
}
