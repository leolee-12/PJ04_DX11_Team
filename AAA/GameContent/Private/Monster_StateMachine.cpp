#include "Monster_StateMachine.h"
#include "Monster.h"
#include "Monster_State.h"

CMonster_StateMachine::CMonster_StateMachine()
{
}

HRESULT CMonster_StateMachine::Initialize(CMonster* pOwner)
{
    if (pOwner == nullptr)
        return E_FAIL;

    m_pOwner = pOwner;

    return S_OK;
}

MONSTER_STATE_TYPE		CMonster_StateMachine::Get_StateType()
{
    if (m_pCurState == nullptr)
        return MONSTER_STATE_TYPE::IDLE;

    return m_pCurState->Get_StateType();
}

_bool CMonster_StateMachine::Change_State(MONSTER_STATE_TYPE eNewState)
{
    CMonster_State* pNextState = Find_State(eNewState);
    if (nullptr == pNextState)
        return false;

    if (m_pCurState == pNextState)
        return true;

    if (nullptr != m_pCurState)
        m_pCurState->Exit(eNewState);

    m_pCurState = pNextState;

    if (nullptr != m_pOwner)
    {
        m_pOwner->Get_BlackBoard().bActionFinished = false;
        m_pOwner->Get_BlackBoard().bCanTransition = m_pCurState->Is_Interruptible();
        m_pOwner->Get_BlackBoard().bMoveLocked = false;
    }

    m_pCurState->Enter();

    return true;
}

void	CMonster_StateMachine::Update_StateMachine(_float fTimeDelta)
{
    if (m_pCurState == nullptr)
        return;

    m_pCurState->Update(fTimeDelta);
}

HRESULT CMonster_StateMachine::Register_State(MONSTER_STATE_TYPE eType, CMonster_State* pState)
{
    if (nullptr == pState)
        return E_FAIL;

    if (Has_State(eType))
    {
        Safe_Release(pState);
        return E_FAIL;
    }

    // StateMachine의 Owner 포인터 주입
    pState->Set_Owner(m_pOwner);

    m_States.emplace(eType, pState);

    return S_OK;
}

_bool CMonster_StateMachine::Has_State(MONSTER_STATE_TYPE eType) const
{
    return m_States.find(eType) != m_States.end();
}

CMonster_State* CMonster_StateMachine::Find_State(MONSTER_STATE_TYPE eNewState)
{
    auto iter = m_States.find(eNewState);

    if (iter == m_States.end())
        return nullptr;

    return iter->second;
}

CMonster_StateMachine* CMonster_StateMachine::Create(CMonster* pOwner)
{
    CMonster_StateMachine* pInstance = new CMonster_StateMachine();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CMonster_StateMachine");
        Safe_Release(pInstance);
    }
    
    return pInstance;
}

void CMonster_StateMachine::Free()
{
    m_pCurState = nullptr;

    for (auto& Pair : m_States)
        Safe_Release(Pair.second);

    m_States.clear();

    __super::Free();
}