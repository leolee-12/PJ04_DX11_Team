#include "Monster_Brain_FSM.h"
#include "Monster.h"

CMonster_Brain_FSM::CMonster_Brain_FSM()
{
}

HRESULT CMonster_Brain_FSM::Initialize()
{
	return S_OK;
}

_bool CMonster_Brain_FSM::Can_Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard) const
{
    if (nullptr == pMonster)
        return false;

    MONSTER_STATE_TYPE eCurState = pMonster->Get_StateType();

    if (eCurState == MONSTER_STATE_TYPE::CAPTURED ||
        eCurState == MONSTER_STATE_TYPE::DEAD)
        return false;

    if (!BlackBoard.bCanTransition)
        return false;


    return true;
}

void CMonster_Brain_FSM::Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    if (nullptr == pMonster)
        return;

    if (!Can_Decide(pMonster, BlackBoard))
        return;

    MONSTER_STATE_TYPE eCurState = pMonster->Get_StateType();

    // 타겟 상실 -> 발견 했었다는 변수 리셋 + IDLE 로 복귀
    if (nullptr == BlackBoard.pTarget)
    {
        m_bSpotted = false; 
        if (eCurState != MONSTER_STATE_TYPE::IDLE && pMonster->Has_State(MONSTER_STATE_TYPE::IDLE))
            pMonster->Change_State(MONSTER_STATE_TYPE::IDLE);
        return;
    }

    // 첫 발견 -> FIND (Find 상태 가진 몬스터만)
    if (!m_bSpotted && pMonster->Has_State(MONSTER_STATE_TYPE::FIND))
    {
        m_bSpotted = true;
        pMonster->Change_State(MONSTER_STATE_TYPE::FIND);
        return;
    }

    Decide_Combat(pMonster, BlackBoard, fTimeDelta);
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
