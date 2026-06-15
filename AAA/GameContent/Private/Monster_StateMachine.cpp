#include "Monster_StateMachine.h"

#include "Monster_State_Idle.h"
#include "Monster_State_Chase.h"

CMonster_StateMachine::CMonster_StateMachine()
{
}

HRESULT CMonster_StateMachine::Initialize(CMonster* pMonster)
{
    m_pMonster = pMonster;

    if (m_pMonster == nullptr)
        return E_FAIL;

    // 임시로 지정
    Change_State(MONSTER_STATE_TYPE::IDLE);

    return S_OK;
}

MONSTER_STATE_TYPE		CMonster_StateMachine::Get_StateType()
{
    if (m_pCurState == nullptr)
        return MONSTER_STATE_TYPE::IDLE;

    return m_pCurState->Get_StateType();
}

void CMonster_StateMachine::Change_State(MONSTER_STATE_TYPE eNewState)
{
    if (m_pCurState && m_pCurState->Get_StateType() == eNewState)
        return;

    if (m_pCurState != nullptr)
    {
        m_pCurState->Exit(m_pMonster);
        Safe_Release(m_pCurState);
    }

    m_pCurState = State_Creator(eNewState);
    if (m_pCurState == nullptr)
        return;

    m_pCurState->Enter(m_pMonster);
}

void	CMonster_StateMachine::Update_StateMachine(_float fTimeDelta)
{
    if (m_pCurState == nullptr)
        return;

    m_pCurState->Update(m_pMonster, fTimeDelta);
}

CMonster_State* CMonster_StateMachine::State_Creator(MONSTER_STATE_TYPE eNewState)
{
    CMonster_State* pState = nullptr;

    switch (eNewState)
    {
    case MONSTER_STATE_TYPE::IDLE:
        pState = CMonster_State_Idle::Create();
        break;
    case MONSTER_STATE_TYPE::CHASE:
        pState = CMonster_State_Chase::Create();
        break;

    case MONSTER_STATE_TYPE::ATTACK:
    case MONSTER_STATE_TYPE::HIT:
    case MONSTER_STATE_TYPE::CAPTURED:
    case MONSTER_STATE_TYPE::DEAD:
    default:
        break;
    }

    return pState;
}

CMonster_StateMachine* CMonster_StateMachine::Create(CMonster* pMonster)
{
    CMonster_StateMachine* pInstance = new CMonster_StateMachine();

    if (FAILED(pInstance->Initialize(pMonster)))
    {
        MSG_BOX("Failed to Created : CMonster_StateMachine");
        Safe_Release(pInstance);
    }
    
    return pInstance;
}

void CMonster_StateMachine::Free()
{
    Safe_Release(m_pCurState);
    __super::Free();
}