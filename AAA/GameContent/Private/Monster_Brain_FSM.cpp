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

    if (eCurState == MONSTER_STATE_TYPE::CAPTURED || eCurState == MONSTER_STATE_TYPE::DEATH ||
        eCurState == MONSTER_STATE_TYPE::KNOCK_OUT || eCurState == MONSTER_STATE_TYPE::KNOCK_BACK_DEATH ||
        eCurState == MONSTER_STATE_TYPE::FALL ||  eCurState == MONSTER_STATE_TYPE::LANDING )
        return false;

    if (!BlackBoard.bCanTransition)
        return false;

    return true;
}

void CMonster_Brain_FSM::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    if (nullptr == m_pOwner)
        return;

    if (!Can_Decide(BlackBoard))
        return;

    MONSTER_STATE_TYPE eCurState = m_pOwner->Get_StateType();

    // 타겟 상실 -> 발견 했었다는 변수 리셋 + IDLE 로 복귀
    if (nullptr == BlackBoard.pTarget)
    {
        m_bSpotted = false; 
        if (eCurState != MONSTER_STATE_TYPE::IDLE && m_pOwner->Has_State(MONSTER_STATE_TYPE::IDLE))
            m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
        return;
    }

    // 첫 발견 -> FIND (Find 상태 가진 몬스터만)
    if (!m_bSpotted && m_pOwner->Has_State(MONSTER_STATE_TYPE::DETECT))
    {
        m_bSpotted = true;
        m_pOwner->Change_State(MONSTER_STATE_TYPE::DETECT);
        return;
    }

    Decide_Internal(BlackBoard, fTimeDelta);
}

void CMonster_Brain_FSM::Free()
{
    __super::Free();
}
