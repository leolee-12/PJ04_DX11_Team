#include "Kirby_StateMachine.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Ability.h"

#include "Kirby_Wait.h"
#include "Kirby_Run.h"
#include "Kirby_Jump.h"
#include "Kirby_Fall.h"
#include "Kirby_Attack.h"
#include "Kirby_Hovering.h"
#include "Kirby_Damaged.h"
#include "Kirby_Guard.h"
#include "Kirby_Slide.h"
#include "Kirby_Dodge.h"
#include "Kirby_Ladder.h"

#include "Kirby_GetAbility.h"
#include "Kirby_AbilityDump.h"
#include "Kirby_GetDeform.h"
#include "Kirby_DeformDump.h"

#include "Kirby_CutSceneGrabbed.h"
#include "Kirby_QTE_Grabbed.h"
#include "Kirby_CarFirstBreakWall.h"
#include "Kirby_DeformCarBridge.h"
#include "Kirby_Clear.h"
#include "Kirby_Dialogue.h"
#include "Kirby_SequenceLock.h"

#include "Kirby_MetaKnightEncounter.h"
#include "Kirby_MetaKnight_QTE.h"

CKirby_StateMachine::CKirby_StateMachine()
    : m_pGameInstance_Proxy(CGameInstance::GetProxy())
{

}

HRESULT CKirby_StateMachine::Initialize(CKirby* pKirby)
{
    m_pKirby = pKirby;
    if (m_pKirby == nullptr)
        return E_FAIL;

    if (FAILED(Init_State()))
        return E_FAIL;

    //Change_State(KIRBY_STATE_TYPE::WAIT);

    if(m_pGameInstance_Proxy->Is_EditMode())
        Change_State(KIRBY_STATE_TYPE::WAIT);
    else
        Change_State(KIRBY_STATE_TYPE::SEQUENCE_LOCK);

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

void CKirby_StateMachine::On_KirbyCollisionEnter_StateMachine(_uint iColliderType, CCollider* pOther)
{
    m_pCurState->On_KirbyCollisionEnter(m_pKirby, iColliderType, pOther);
}

void CKirby_StateMachine::On_KirbyCollisionStay_StateMachine(_uint iColliderType, CCollider* pOther)
{
    m_pCurState->On_KirbyCollisionStay(m_pKirby, iColliderType, pOther);
}

void CKirby_StateMachine::On_KirbyCollisionExit_StateMachine(_uint iColliderType, CCollider* pOther)
{
    m_pCurState->On_KirbyCollisionExit(m_pKirby, iColliderType, pOther);
}

void CKirby_StateMachine::On_Damaged_KirbyStateMachine(const ATTACK_INFO& tInfo)
{
    m_pCurState->On_Damaged_KirbyState(m_pKirby, tInfo);
}

void CKirby_StateMachine::Request_Attachment_StateMachine(const KIRBY_ATTACHMENT_BEGIN_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_ATTACHMENT_CONTEXT::GORILLA_SCENE:
        {
            Change_State(KIRBY_STATE_TYPE::CUTSCENE_GRABBED);
            break;
        }
        case KIRBY_ATTACHMENT_CONTEXT::GORILLA_COMBAT:
        {
            Change_State(KIRBY_STATE_TYPE::QTE_GRABBED);
            break;
        }
        case KIRBY_ATTACHMENT_CONTEXT::DEFORM_CAR_GET_FIRST:
        {
            Change_State(KIRBY_STATE_TYPE::CAR_FIRST_BREAK_WALL);
            break;
        }
        default:
        {
            MSG_BOX("Event Miss: CKirby_StateMachine");
            return;
        }
    }

    m_pCurState->Request_Attachment(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_Attachment_End_StateMachine(const KIRBY_ATTACHMENT_END_DESC* pDesc)
{
    m_pCurState->Request_Attachment_End(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_PositionSync_StateMachine(const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_CONTEXT::CAR_BRIDGE:
        {
            Change_State(KIRBY_STATE_TYPE::DEFORM_CAR_BRIDGE);
            break;
        }
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_LOOKAROUND:
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_INTRO:
        {
            Change_State(KIRBY_STATE_TYPE::METAKNIGHT_ENCOUNTER);
            break;
        }
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_UPPERCALIBUR:
        {
            Change_State(KIRBY_STATE_TYPE::METAKNIGHT_QTE);
            break;
        }
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_LOCKING_WIN:
        {
            break;
        }
        case KIRBY_POSITION_SYNC_CONTEXT::_COUNT:
        default:        
        {
            MSG_BOX("Request_PositionSync_StateMachine Error: CKirby_StateMachine");
            return;
        }
    }

    m_pCurState->Request_PositionSync(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_PositionSync_End_StateMachine(const KIRBY_POSITION_SYNC_END_DESC* pDesc)
{
    m_pCurState->Request_PositionSync_End(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_ClearStage_StateMachine(const CUTSCENE_STAGECLEAR* pDesc)
{
    if (m_pCurState->Get_StateType() != KIRBY_STATE_TYPE::STAGE_CLEAR)
        Change_State(KIRBY_STATE_TYPE::STAGE_CLEAR);

    m_pCurState->Request_StageClear(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_Dialogue_StateMachine(const SEQUENCE_KIRBY_WARP_DESC* pDesc)
{
    if (m_pCurState->Get_StateType() != KIRBY_STATE_TYPE::DIALOGUE)
        Change_State(KIRBY_STATE_TYPE::DIALOGUE);

    m_pCurState->Request_Dialogue(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_DialogueAnim_StateMachine(const SEQUENCE_KIRBY_ANIM_DESC* pDesc)
{
    if (m_pCurState->Get_StateType() != KIRBY_STATE_TYPE::DIALOGUE)
    {
        MSG_BOX("Is Not Dialogue State : CKirby_StateMachine");
        return;
    }

    m_pCurState->Request_DialogueAnim(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_SequenceLock_StateMachine(const KIRBY_LEVEL_SLEEP_DESC* pDesc)
{
    m_pCurState->Cleanup_ForLevelTransition(m_pKirby, pDesc);

    if (m_pCurState->Get_StateType() != KIRBY_STATE_TYPE::SEQUENCE_LOCK)
        Change_State(KIRBY_STATE_TYPE::SEQUENCE_LOCK);

    m_pCurState->Request_SequenceLock(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_SequenceLock_End_StateMachine(const KIRBY_LEVEL_SPAWN_DESC* pDesc)
{
    m_pCurState->Request_SequenceLock_End(m_pKirby, pDesc);
}

void CKirby_StateMachine::Request_MetaKnight_ParryBegin_StateMachine()
{
    Change_State(KIRBY_STATE_TYPE::METAKNIGHT_QTE);
    m_pCurState->Request_MetaKnight_ParryBegin(m_pKirby);
}

void CKirby_StateMachine::Get_EssenceBubble(COPY_ABILITY_TYPE eNewAbility)
{
    if (m_pKirby->Has_Deform())
        return;

    KIRBY_STATE_TYPE eCurState = Get_StateType();
    if (eCurState == KIRBY_STATE_TYPE::GET_ABILITY || eCurState == KIRBY_STATE_TYPE::GET_DEFORM ||
        eCurState == KIRBY_STATE_TYPE::ABILITY_DUMP)
        return;

    CKirby_Ability* pAbility = m_pKirby->Get_KirbyAbility();
    COPY_ABILITY_TYPE eCurAbilityType = pAbility->Get_AbilityType();
    if (eCurAbilityType == eNewAbility)
        return;

    m_pKirby->Request_ChangeKirbyAbility(eNewAbility);
    m_pKirby->Change_State(KIRBY_STATE_TYPE::GET_ABILITY, GETABILITY_STATE_FLAG::ESSENCE);
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

    if (FAILED(Register_State(KIRBY_STATE_TYPE::WAIT, CKirby_Wait::Create())))                                  return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::RUN, CKirby_Run::Create())))                                    return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::JUMP, CKirby_Jump::Create())))                                  return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::FALL, CKirby_Fall::Create())))                                  return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::ATTACK, CKirby_Attack::Create())))                              return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::HOVERING, CKirby_Hovering::Create())))                          return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::DAMAGED, CKirby_Damaged::Create())))                            return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::GUARD, CKirby_Guard::Create())))                                return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::SLIDE, CKirby_Slide::Create())))                                return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::DODGE, CKirby_Dodge::Create())))                                return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::LADDER, CKirby_Ladder::Create())))                              return E_FAIL;


    if (FAILED(Register_State(KIRBY_STATE_TYPE::GET_ABILITY, CKirby_GetAbility::Create())))                     return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::ABILITY_DUMP, CKirby_AbilityDump::Create())))                   return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::GET_DEFORM, CKirby_GetDeform::Create())))                       return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::DEFORM_DUMP, CKirby_DeformDump::Create())))                     return E_FAIL;

    if (FAILED(Register_State(KIRBY_STATE_TYPE::QTE_GRABBED, CKirby_QTE_Grabbed::Create())))                    return E_FAIL;

    if (FAILED(Register_State(KIRBY_STATE_TYPE::CUTSCENE_GRABBED, CKirby_CutSceneGrabbed::Create())))           return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::CAR_FIRST_BREAK_WALL, CKirby_CarFirstBreakWall::Create())))     return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::DEFORM_CAR_BRIDGE, CKirby_DeformCarBridge::Create())))          return E_FAIL;

    if (FAILED(Register_State(KIRBY_STATE_TYPE::METAKNIGHT_ENCOUNTER, CKirby_MetaKnightEncounter::Create())))   return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::METAKNIGHT_QTE, CKirby_MetaKnight_QTE::Create())))              return E_FAIL;

    if (FAILED(Register_State(KIRBY_STATE_TYPE::STAGE_CLEAR, CKirby_Clear::Create())))                          return E_FAIL;
    if (FAILED(Register_State(KIRBY_STATE_TYPE::DIALOGUE, CKirby_Dialogue::Create())))                          return E_FAIL;

    if (FAILED(Register_State(KIRBY_STATE_TYPE::SEQUENCE_LOCK, CKirby_SequenceLock::Create())))                 return E_FAIL;

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

    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}
