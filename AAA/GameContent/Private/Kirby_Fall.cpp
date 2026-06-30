#include "Kirby_Fall.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

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

void CKirby_Fall::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    Change_FallState(FALL_STATE::FALLING);

    if(pKirby->Has_Deform())
        pKirby->Get_KirbyDeform()->Play_DeformAni(pKirby, DEFORM_ANI::FALL);
    else
        pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::FALL);
}

void CKirby_Fall::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_FallState(pKirby);
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
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
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
            if (pKirby->Has_Deform())
            {
                CKirby_Deform* pDeform = pKirby->Get_KirbyDeform();
                if (pCommand->IsDown())
                    pDeform->Enter_Attack_KeyDown(pKirby);
                else if (pCommand->IsPress())
                    pDeform->Enter_Attack_KeyPress(pKirby);
                else if (pCommand->IsUp())
                    pDeform->Enter_Attack_KeyUp(pKirby);
            }
            else
            {
                CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
                if (pAbility->Can_Attack(KIRBY_ATTACK_LOCATION::GROUND))
                {
                    if (pCommand->IsDown())
                        pAbility->Enter_Attack_KeyDown(pKirby);
                    else if (pCommand->IsPress())
                        pAbility->Enter_Attack_KeyPress(pKirby);
                    else if (pCommand->IsUp())
                        pAbility->Enter_Attack_KeyUp(pKirby);
                }
            }

            return true;
        }
    }

    return false;
}

void CKirby_Fall::Update_FallState(CKirby* pKirby)
{
    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CMovement_Child* pMovement = pKirby->Get_Movement();

    switch (m_eFallState)
    {
        case FALL_STATE::LAND_START:
            if (pKirby->Has_Deform())
            {
                CKirby_Deform* pKirbyDeform = pKirby->Get_KirbyDeform();
                CKirby_Deform_Model* pDeformModel = pKirby->Get_DeformPart_Model(pKirbyDeform->Get_DeformType());
                if (pDeformModel->Get_Animator()->Is_Finished())
               
                    Transition_Wait_OR_Run(pKirby);
            }
            else
            {
                CAnimator* pAnimator = pKirby_Body->Get_Animator();
                if (pAnimator->Is_Finished())
                    Transition_Wait_OR_Run(pKirby);
            }   
            break;

        case FALL_STATE::FALLING:
            if (pMovement->Is_Grounded() == true)
            {
                if (pKirby->Has_Deform())
                {
                    pKirby->Get_KirbyDeform()->Play_DeformAni(pKirby, DEFORM_ANI::LANDING);
                }
                else
                {
                    pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::LANDING);

                    if (pKirby->Get_Body()->Get_KirbyBody() != KIRBY_BODY_STATE::STUFFED)
                        pKirby_Body->Set_KirbyEye(KIRBY_EYE_STATE::CLOSE);
                }

                Change_FallState(FALL_STATE::LAND_START);
            }
            break;
    }
}

void CKirby_Fall::Change_FallState(FALL_STATE eNewState)
{
    if (m_eFallState == eNewState)
        return;

    m_eFallState = eNewState;
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