#include "Monster_Brain_FSM.h"
#include "Monster.h"

CMonster_Brain_FSM::CMonster_Brain_FSM()
{
}

HRESULT CMonster_Brain_FSM::Initialize()
{
	return S_OK;
}

void CMonster_Brain_FSM::Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    MONSTER_STATE_TYPE eCur = pMonster->Get_StateType();

    // [범주 1] - Interrupted 는  제외
    if (eCur == MONSTER_STATE_TYPE::HIT ||
        eCur == MONSTER_STATE_TYPE::CAPTURED ||
        eCur == MONSTER_STATE_TYPE::DEAD)
        return;

    // [범주 2] - '완료'까지 유지 하는 상태 - 끝나기 전에는 안 건드림
    if (eCur == MONSTER_STATE_TYPE::ATTACK)
    {
        if (BlackBoard.bActionFinished)
        {
            pMonster->Change_State(MONSTER_STATE_TYPE::RETREAT);
            return;
        }
        else
            return;
    }

    // [범주 3] - 자유 재평가
    if (BlackBoard.bCanSeeTarget && BlackBoard.bActionFinished)
        pMonster->Change_State(MONSTER_STATE_TYPE::ATTACK);     // 임시로 작성
    else
        pMonster->Change_State(MONSTER_STATE_TYPE::IDLE);       // 다 빠져나오면 IDLE 회귀
}

CMonster_Brain_FSM*  CMonster_Brain_FSM::Create()
{
    CMonster_Brain_FSM* pInstance = new CMonster_Brain_FSM();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CMonster_Brain_FSM");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMonster_Brain_FSM::Free()
{
    __super::Free();
}
