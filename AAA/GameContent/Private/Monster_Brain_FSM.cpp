#include "Monster_Brain_FSM.h"
#include "Monster.h"

CMonster_Brain_FSM::CMonster_Brain_FSM()
{
}

HRESULT CMonster_Brain_FSM::Initialize(CMonster* pOwner)
{
    if (pOwner == nullptr)
        return E_FAIL;

    m_pOwner = pOwner;

	return S_OK;
}

_bool CMonster_Brain_FSM::Can_Decide(const MONSTER_BLACKBOARD& BlackBoard) const
{
    if (nullptr == m_pOwner)
        return false;

    MONSTER_STATE_TYPE eCurState = m_pOwner->Get_StateType();

    if (eCurState == MONSTER_STATE_TYPE::CAPTURED ||
        eCurState == MONSTER_STATE_TYPE::DEATH)
        return false;

    if (!BlackBoard.bCanTransition)
        return false;

    return true;
}

void CMonster_Brain_FSM::Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    if (nullptr == pMonster)
        return;

    if (!Can_Decide(BlackBoard))
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
    if (!m_bSpotted && pMonster->Has_State(MONSTER_STATE_TYPE::DETECT))
    {
        m_bSpotted = true;
        pMonster->Change_State(MONSTER_STATE_TYPE::DETECT);
        return;
    }

    Decide_Internal(BlackBoard, fTimeDelta);
}

void CMonster_Brain_FSM::Free()
{
    __super::Free();
}
