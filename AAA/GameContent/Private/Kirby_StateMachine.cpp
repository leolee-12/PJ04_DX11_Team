#include "Kirby_StateMachine.h"

#include "GameInstance.h"

#include "Kirby.h"

#include "Kirby_Wait.h"
#include "Kirby_Run.h"
#include "Kirby_Jump.h"
#include "Kirby_Fall.h"
#include "Kirby_Attack.h"
#include "Kirby_Hovering.h"
#include "Kirby_GetAbility.h"
#include "Kirby_AbilityDump.h"
#include "Kirby_Damaged.h"
#include "Kirby_Guard.h"
#include "Kirby_Slide.h"
#include "Kirby_Dodge.h"
#include "Kirby_Ladder.h"
#include "Kirby_GetDeform.h"

#include "Kirby_CutSceneGrabbed.h"
#include "Kirby_QTE_Grabbed.h"
#include "Kirby_CarFirstBreakWall.h"
#include "Kirby_Clear.h"

CKirby_StateMachine::CKirby_StateMachine()
{

}

HRESULT CKirby_StateMachine::Initialize(CKirby* pKirby)
{
    m_pKirby = pKirby;
    if (m_pKirby == nullptr)
        return E_FAIL;

    if (FAILED(Init_State()))
        return E_FAIL;

    Change_State(KIRBY_STATE_TYPE::WAIT);

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_StateMachine::Get_StateType()
{
    return m_pCurState->Get_StateType();
}

void CKirby_StateMachine::Change_State(KIRBY_STATE_TYPE eNewstate, _int iFlag)
{
    if (m_pCurState && m_pCurState->Get_StateType() == eNewstate)
        return;

    if(m_pCurState != nullptr)
        m_pCurState->Exit(m_pKirby);

    m_pKirby->Apply_ChangeKirbyAbility();

    m_pCurState = Find_State(eNewstate);

    if (m_pCurState == nullptr)
        return;

    m_pCurState->Enter(m_pKirby, iFlag);
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

void CKirby_StateMachine::On_Damaged_KirbyStateMachine(const ATTACK_INFO& tInfo)
{
    m_pCurState->On_Damaged_KirbyState(m_pKirby, tInfo);
}

void CKirby_StateMachine::Request_GrabState_StateMachine(KIRBY_ATTACHMENT_CONTEXT eType)
{
    m_pCurState->Request_GrabState(m_pKirby, eType);
}

void CKirby_StateMachine::Request_ReleaseGrabState_StateMachine(KIRBY_ATTACHMENT_END_REASON eType)
{
    m_pCurState->Request_ReleaseGrabState(m_pKirby, eType);
}

void CKirby_StateMachine::Request_ClearStage_StateMachine(const CUTSCENE_STAGECLEAR* pDesc)
{
    if (m_pCurState->Get_StateType() != KIRBY_STATE_TYPE::STAGE_CLEAR)
        Change_State(KIRBY_STATE_TYPE::STAGE_CLEAR);

    m_pCurState->Request_StageClear(m_pKirby, pDesc);
}

_bool CKirby_StateMachine::Ignore_TimeScale_StateMachine()
{
    return m_pCurState->Ignore_TimeScale();
}

HRESULT CKirby_StateMachine::Init_State()
{
    auto Register_State = [this](KIRBY_STATE_TYPE eType, CKirby_State* pNewState) -> HRESULT
        {
            if (pNewState == nullptr)
                return E_FAIL;

            m_States[eType] = pNewState;

            return S_OK;
        };

    if (FAILED(Register_State(KIRBY_STATE_TYPE::WAIT, CKirby_Wait::Create())))                              return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::RUN, CKirby_Run::Create())))                                return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::JUMP, CKirby_Jump::Create())))                              return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::FALL, CKirby_Fall::Create())))                              return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::ATTACK, CKirby_Attack::Create())))                          return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::HOVERING, CKirby_Hovering::Create())))                      return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::GET_ABILITY, CKirby_GetAbility::Create())))                 return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::ABILITY_DUMP, CKirby_AbilityDump::Create())))               return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::DAMAGED, CKirby_Damaged::Create())))                        return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::GUARD, CKirby_Guard::Create())))                            return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::SLIDE, CKirby_Slide::Create())))                            return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::DODGE, CKirby_Dodge::Create())))                            return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::LADDER, CKirby_Ladder::Create())))                          return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::GET_DEFORM, CKirby_GetDeform::Create())))                   return E_FAIL;


    if (FAILED(Register_State(KIRBY_STATE_TYPE::CUTSCENE_GRABBED, CKirby_CutSceneGrabbed::Create())))       return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::QTE_GRABBED, CKirby_QTE_Grabbed::Create())))                return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::CAR_FIRST_BREAK_WALL, CKirby_CarFirstBreakWall::Create()))) return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::STAGE_CLEAR, CKirby_Clear::Create())))                      return E_FAIL;

    return S_OK;
}

CKirby_State* CKirby_StateMachine::Find_State(KIRBY_STATE_TYPE eNewstate)
{
    auto iter = m_States.find(eNewstate);

    if (iter != m_States.end())
        return iter->second;

    return nullptr;
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
    m_pCurState = nullptr;

    for (auto& pair : m_States)
        Safe_Release(pair.second);
    m_States.clear();

    __super::Free();
}
