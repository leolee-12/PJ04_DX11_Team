#include "Monster_StateMachine.h"

#include "Monster_State_Idle.h"
#include "Monster_State_Chase.h"
#include "Monster_State_Attack.h"

CMonster_StateMachine::CMonster_StateMachine()
{
}

HRESULT CMonster_StateMachine::Initialize(CMonster* pMonster)
{
    m_pMonster = pMonster;

    if (m_pMonster == nullptr)
        return E_FAIL;

    if (FAILED(Init_State()))
        return E_FAIL;

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
        m_pCurState->Exit(m_pMonster);

    m_pCurState = Find_State(eNewState);
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

HRESULT CMonster_StateMachine::Init_State()
{
    auto Register_State = [this](MONSTER_STATE_TYPE eType, CMonster_State* pNewState) -> HRESULT
        {
            if (nullptr == pNewState)
                return E_FAIL;

            m_States[eType] = pNewState;
            return S_OK;
        };

    if (FAILED(Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create())))
        return E_FAIL;

    if (FAILED(Register_State(MONSTER_STATE_TYPE::CHASE, CMonster_State_Chase::Create())))
        return E_FAIL;

    if (FAILED(Register_State(MONSTER_STATE_TYPE::ATTACK, CMonster_State_Attack::Create())))
        return E_FAIL;

    return S_OK;
}

CMonster_State* CMonster_StateMachine::Find_State(MONSTER_STATE_TYPE eNewState)
{
    auto iter = m_States.find(eNewState);

    if (iter == m_States.end())
        return nullptr;

    return iter->second;
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
    m_pCurState = nullptr;

    for (auto& Pair : m_States)
        Safe_Release(Pair.second);

    m_States.clear();

    __super::Free();
}