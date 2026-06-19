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

	pMonster->Get_BlackBoard().bActionFinished = false;
	pMonster->Get_BlackBoard().bCanTransition = true;
	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::CHASE);
}

void CMonster_State_Chase::Update(CMonster* pMonster, _float fTimeDelta)
{
	if (nullptr == pMonster)
		return;

	const MONSTER_BLACKBOARD& BlackBoard = pMonster->Get_BlackBoard();

	if (BlackBoard.pTarget == nullptr)
		return;

	const _float3& vMoveDir = BlackBoard.vDirToTargetXZ;

	if (XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&vMoveDir))) < 0.0001f)
		return;

	// 플레이어가 공중에 일정 거리 이상 떴을 때 CHASE 멈추기
	if (fabsf(BlackBoard.fHeightToTarget > 1.5f))
		return;

	pMonster->Add_MoveDir(vMoveDir);
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
