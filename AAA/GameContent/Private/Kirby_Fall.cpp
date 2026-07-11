#include "Kirby_Fall.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

#include "Movement_Child.h"

CKirby_Fall::CKirby_Fall()
{
}

HRESULT CKirby_Fall::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Fall::Get_StateType()
{
    return KIRBY_STATE_TYPE::FALL;
}

void CKirby_Fall::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    m_eFallState = FALL_STATE::FALL_STATE_END;

    switch (static_cast<FALL_STATE_FLAG>(iFlag))
    {
        case FALL_STATE_FLAG::FALL_DIRECT:
        {
            Change_FallState(pKirby, FALL_STATE::FALLING);
            break;
        }
        case FALL_STATE_FLAG::PLAY_JUMP_END_L:
        {
            m_bLeft = true;
            Change_FallState(pKirby, FALL_STATE::JUMP_END);
            break;
        }
        case FALL_STATE_FLAG::PLAY_JUMP_END_R:
        {
            m_bLeft = false;
            Change_FallState(pKirby, FALL_STATE::JUMP_END);
            break;
        }
    }
}

void CKirby_Fall::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_FallState(pKirby, fTimeDelta);

    m_bGuardReserved = false;
}

void CKirby_Fall::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    if (pKirby->Get_Body()->Get_KirbyBody() != KIRBY_BODY_STATE::STUFFED)
    {
        CKirby_Body* pKirby_Body = pKirby->Get_Body();
        pKirby_Body->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);
    }
}

_bool CKirby_Fall::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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
            return true;
        }
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        {
            if (!pCommand->IsPress())
                return false;

            if (Try_Transition_Ladder_CommandDown(pKirby))
                return true;

            Handle_MoveCommand(pKirby, pCommand);
            return true;
        }
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            Handle_MoveCommand(pKirby, pCommand);
            return true;
        }
        // Hovering
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            CMovement_Child* pMovementCom = pKirby->Get_Movement();
            if (pMovementCom->Is_Grounded() == true)
                pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
            else if (pKirby->Get_Body()->Get_KirbyBody() != KIRBY_BODY_STATE::STUFFED && !pKirby->Has_Deform())
                pKirby->Change_State(KIRBY_STATE_TYPE::HOVERING);

            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            CKirby_AttackMode* pAttackMode = pKirby->Get_ActiveAttackMode();
            if (pAttackMode->Can_Attack(KIRBY_ATTACK_LOCATION::AIR))
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

            m_bGuardReserved = true;
            return true;
        }
    }

    return false;
}

void CKirby_Fall::Change_FallState(CKirby* pKirby, FALL_STATE eNewState)
{
    if (m_eFallState == eNewState)
        return;

    Exit_FallState(pKirby, m_eFallState);

    m_eFallState = eNewState;

    Enter_FallState(pKirby, m_eFallState);
}

void CKirby_Fall::Enter_FallState(CKirby* pKirby, FALL_STATE eState)
{
    switch (eState)
    {
        case FALL_STATE::JUMP_END:
        {
            if (pKirby->Has_Deform())
            {
                MSG_BOX("Bug 1: CKirby_Fall");
            }
            else
            {
                CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
                ABILITY_ANI eJumpEndAni = m_bLeft ? ABILITY_ANI::JUMP_END_L : ABILITY_ANI::JUMP_END_R;
                pAbility->Play_AbilityAni(pKirby, eJumpEndAni);
            }
            break;
        }
        case FALL_STATE::FALLING:
        {
            if (pKirby->Has_Deform())
                pKirby->Get_KirbyDeform()->Play_DeformAni(pKirby, DEFORM_ANI::FALL);
            else
                pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::FALL);
            break;
        }
        case FALL_STATE::LAND_START:
        {
            if (pKirby->Has_Deform())
            {
                pKirby->Get_KirbyDeform()->Play_DeformAni(pKirby, DEFORM_ANI::LANDING);
            }
            else
            {
                pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::LANDING);

                CKirby_Body* pKirby_Body = pKirby->Get_Body();
                if (pKirby_Body->Get_KirbyBody() != KIRBY_BODY_STATE::STUFFED)
                    pKirby_Body->Set_KirbyEye(KIRBY_EYE_STATE::CLOSE);
            }
            break;
        }
        case FALL_STATE::FALL_STATE_END:
            break;
    }
}

void CKirby_Fall::Update_FallState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eFallState)
    {
        case FALL_STATE::JUMP_END:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            if (pMovement->Is_Grounded())
            {
                Change_FallState(pKirby, FALL_STATE::LAND_START);
                return;
            }

            if (pKirby->Has_Deform())
            {
                MSG_BOX("Bug 2: CKirby_Fall");
            }
            else
            {
                CKirby_Body* pKirby_Body = pKirby->Get_Body();
                CAnimator* pAnimator = pKirby_Body->Get_Animator();
                if (pAnimator->Is_Finished())
                    Change_FallState(pKirby, FALL_STATE::FALLING);
            }
      
            break;
        }
        case FALL_STATE::FALLING:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();

            if (pMovement->Is_Grounded() == false)
                return;

            Change_FallState(pKirby, FALL_STATE::LAND_START);
            break;
        }
        case FALL_STATE::LAND_START:
        {
            if (m_bGuardReserved)
            {
                pKirby->Change_State(KIRBY_STATE_TYPE::GUARD);
                return;
            }

            CAnimator* pAnimator{};

            if (pKirby->Has_Deform())
            {
                CKirby_Deform* pKirbyDeform = pKirby->Get_KirbyDeform();
                DEFORM_TYPE eDeformType = pKirbyDeform->Get_DeformType();
                CKirby_Deform_Model* pDeformModel = pKirby->Get_DeformPart_Model(eDeformType);
                pAnimator = pDeformModel->Get_Animator();
            }
            else
            {
                CKirby_Body* pKirby_Body = pKirby->Get_Body();
                pAnimator = pKirby_Body->Get_Animator();
            }

            if (pAnimator->Is_Finished())
            {
                Transition_Wait_OR_Run(pKirby);
                return;
            }
            break;
        }
        case FALL_STATE::FALL_STATE_END:
            break;
    }
}

void CKirby_Fall::Exit_FallState(CKirby* pKirby, FALL_STATE eState)
{
    switch (eState)
    {
        case FALL_STATE::JUMP_END:
            break;
        case FALL_STATE::FALLING:
            break;
        case FALL_STATE::LAND_START:
            break;
        case FALL_STATE::FALL_STATE_END:
            break;
    }
}

CKirby_Fall* CKirby_Fall::Create()
{
    CKirby_Fall* pInstance = new CKirby_Fall();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Fall");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Fall::Free()
{
    __super::Free();
}