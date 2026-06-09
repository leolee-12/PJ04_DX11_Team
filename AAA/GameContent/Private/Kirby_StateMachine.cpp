#include "Kirby_StateMachine.h"

#include "GameInstance.h"

#include "Kirby_Wait.h"
#include "Kirby_Run.h"
#include "Kirby_Jump.h"
#include "Kirby_Inhale.h"
#include "Kirby_Attack.h"

CKirby_StateMachine::CKirby_StateMachine()
{

}

HRESULT CKirby_StateMachine::Initialize(CKirby* pKirby)
{
    m_pKirby = pKirby;
    if (m_pKirby == nullptr)
        return E_FAIL;

    Change_State(KIRBY_STATE_TYPE::WAIT);

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_StateMachine::Get_StateType()
{
    return m_pCurState->Get_StateType();
}

void CKirby_StateMachine::Change_State(KIRBY_STATE_TYPE eNewstate)
{
    if (m_pCurState && m_pCurState->Get_StateType() == eNewstate)
        return;

    if(m_pCurState != nullptr)
    {
        m_pCurState->Exit(m_pKirby);
        Safe_Release(m_pCurState);
    }

    m_pCurState = State_Creator(eNewstate);

    if (m_pCurState == nullptr)
        return;

    m_pCurState->Enter(m_pKirby);

}

void CKirby_StateMachine::Update_StateMachine(const _float fTimeDelta)
{
    if (m_pCurState == nullptr)
        return;

    m_pCurState->Update(m_pKirby, fTimeDelta);
}

void CKirby_StateMachine::Handle_Command(CKirby_Command* pCommand)
{
    m_pCurState->Handle_Command(m_pKirby, pCommand);
}

CKirby_State* CKirby_StateMachine::State_Creator(KIRBY_STATE_TYPE eNewstate)
{
    CKirby_State* pState{};

    switch (eNewstate)
    {
        case KIRBY_STATE_TYPE::WAIT:        pState = CKirby_Wait::Create();     break;
        case KIRBY_STATE_TYPE::RUN:         pState = CKirby_Run::Create();      break;
        case KIRBY_STATE_TYPE::JUMP:        pState = CKirby_Jump::Create();     break;
        case KIRBY_STATE_TYPE::INHALE:      pState = CKirby_Inhale::Create();   break;
        case KIRBY_STATE_TYPE::ATTACK:      pState = CKirby_Attack::Create();   break;  
    }

    return pState;
}

CKirby_StateMachine* CKirby_StateMachine::Create(CKirby* pKirby)
{
    CKirby_StateMachine* pInstance = new CKirby_StateMachine();

    if (FAILED(pInstance->Initialize(pKirby)))
    {
        MSG_BOX("Failed to Created: CKirby_StateMachine");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_StateMachine::Free()
{
    Safe_Release(m_pCurState);

    __super::Free();
}
