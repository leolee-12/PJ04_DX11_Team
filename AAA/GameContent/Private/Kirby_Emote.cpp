#include "Kirby_Emote.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

#include "Kirby_Deform.h"

#include "WaddleDee.h"

CKirby_Emote::CKirby_Emote()
{
}

HRESULT CKirby_Emote::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Emote::Get_StateType()
{
    return KIRBY_STATE_TYPE::EMOTE;
}

void CKirby_Emote::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    m_eEmoteStateFlag = iFlag;
    m_bInteracted = false;

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    switch (iFlag)
    {
        case EMOTE_STATE_FLAG::EMOTE_TOP:
            pAnimator->Play("EmoteWaveHand", false, true, 0.1f, 1.5f);
            m_fInteractedRatio = 0.2f;
            break;
        case EMOTE_STATE_FLAG::EMOTE_DOWN:
            pAnimator->Play("WaitSit", false, true, 0.1f, 1.5f);
            break;
        case EMOTE_STATE_FLAG::EMOTE_LEFT:
            pAnimator->Play("WaitYay", false, true, 0.1f, 1.5f);
            m_fInteractedRatio = 0.1f;
            break;
        case EMOTE_STATE_FLAG::EMOTE_RIGHT:
            pAnimator->Play("LookAround", false, true, 0.1f, 1.5f);
            break;
        default:
            MSG_BOX("Bug: CKirby_Emote");
            Transition_Fall_OR_Wait_OR_Run(pKirby);
            return;
    }

    if(m_pQueryBox == nullptr)
    {
        CCollider::COLLIDER_DESC tBoxDesc{};
        tBoxDesc.pOwner = pKirby;
        tBoxDesc.vCenter = { 0.f, 1.f, 0.f };
        tBoxDesc.vSize = { 3.f, 2.f, 3.f };

        m_pQueryBox = static_cast<CCollider*>(m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::COMPONENT,
            Collider_AABB.iLevelID, Collider_AABB.szProtoTag, &tBoxDesc));
    }
}

void CKirby_Emote::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    if (!pKirby->Get_Movement()->Is_Grounded())
    {
        pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
        return;
    }


    const _bool bReactionEmote = m_eEmoteStateFlag == EMOTE_STATE_FLAG::EMOTE_TOP ||
        m_eEmoteStateFlag == EMOTE_STATE_FLAG::EMOTE_LEFT;

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    _float fRatio = pAnimator->Get_Progress();
    if (bReactionEmote && fRatio >= m_fInteractedRatio && !m_bInteracted && m_pQueryBox)
    {
        m_pQueryBox->Update(XMLoadFloat4x4(pKirby->Get_Transform()->Get_WorldMatrixPtr()));

        vector<CCollider*> Hits;
        m_pGameInstance_Proxy->Query_Overlap(m_pQueryBox, ETOUI(COLLISION_LAYER::ENV_HURT), &Hits);

        for (CCollider* pCollider : Hits)
        {
            if (pCollider == nullptr)
                continue;

            CGameObject* pGameObject = pCollider->Get_Owner();
            if (pGameObject == nullptr)
                continue;

            CWaddleDee* pWaddleDee =dynamic_cast<CWaddleDee*>(pGameObject);
            if (pWaddleDee == nullptr)
                continue;

            const CWaddleDee::WADDLEDEE_EMOTE eEmote
                = m_eEmoteStateFlag == EMOTE_STATE_FLAG::EMOTE_TOP
                ? CWaddleDee::WADDLEDEE_EMOTE::WAVE
                : CWaddleDee::WADDLEDEE_EMOTE::YAY;

            pWaddleDee->React_Emote(eEmote);
        }

        m_bInteracted = true;
    }

    if (pAnimator->Is_Finished())
    {
        Transition_Fall_OR_Wait_OR_Run(pKirby);
        return;
    }
}

void CKirby_Emote::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_Emote::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        {
            if (!pCommand->IsPress())
                return false;

            if (Try_Transition_Ladder_CommandUp(pKirby))
                return true;

            Handle_MoveCommand(pKirby, pCommand);
            pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
            return true;
        }
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        {
            if (!pCommand->IsPress())
                return false;

            if (Try_Transition_Ladder_CommandDown(pKirby))
                return true;

            Handle_MoveCommand(pKirby, pCommand);
            pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
            return true;
        }
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            Handle_MoveCommand(pKirby, pCommand);
            pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
            return true;
        }
        // Jump Down
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            CKirby_AttackMode* pAttackMode = pKirby->Get_ActiveAttackMode();
            if (pAttackMode->Can_Attack(KIRBY_ATTACK_LOCATION::GROUND))
            {
                if (pCommand->IsDown())
                    pAttackMode->Enter_Attack_KeyDown(pKirby);
                else if (pCommand->IsPress())
                    pAttackMode->Enter_Attack_KeyPress(pKirby);
                else if (pCommand->IsUp())
                    pAttackMode->Enter_Attack_KeyUp(pKirby);
            }

            return true;
        }
        // Guard
        case KIRBY_COMMAND_TYPE::GUARD:
        {
            if (!pCommand->IsPress())
                return false;

            if (pKirby->Has_Deform())
                return true;

            pKirby->Change_State(KIRBY_STATE_TYPE::GUARD);
            return true;
        }
    }

    return false;
}

CKirby_Emote* CKirby_Emote::Create()
{
    CKirby_Emote* pInstance = new CKirby_Emote();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Emote");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Emote::Free()
{
    Safe_Release(m_pQueryBox);

    __super::Free();
}
