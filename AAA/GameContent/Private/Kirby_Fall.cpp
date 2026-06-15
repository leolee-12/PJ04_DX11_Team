#include "Kirby_Fall.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

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

    // Fall State
    m_eFallingState = FALL_STATE::FALLING;
}

void CKirby_Fall::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CMovement_Child* pMovementCom = pKirby->Get_Movement();

    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();

    // 착지 후 애니메이션
    if (m_eFallingState == FALL_STATE::FALLING &&
        pMovementCom->Is_Grounded() == true)
    {
        // Fall State
        m_eFallingState = FALL_STATE::LAND_START;
        // Ani
        pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::LANDING));
        // Eye
        pKirby_Body->Set_Eye(KIRBY_EYE_STATE::CLOSE);
    }
    // 애니메이션 끝나면 Wait or Run
    else if (m_eFallingState == FALL_STATE::LAND_START &&
        pAnimator->Is_Finished())
    {
        Transition_Wait_OR_Run(pKirby);
    }
}

void CKirby_Fall::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    // Ani
    CAnimator* pAnimator = pKirby_Body->Get_Animator();
    // Eye
    pKirby_Body->Set_Eye(KIRBY_EYE_STATE::IDLE);
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
            else
                pKirby->Change_State(KIRBY_STATE_TYPE::HOVERING);

            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
            if (pAbility->Can_Attack(KIRBY_ATTACK_LOCATION::AIR))
            {
                if (pCommand->IsDown())
                    pAbility->Enter_Attack_KeyDown(pKirby);
                else if (pCommand->IsPress())
                    pAbility->Enter_Attack_KeyPress(pKirby);
                else if (pCommand->IsUp())
                    pAbility->Enter_Attack_KeyUp(pKirby);
            }

            return true;
        }
    }

    return false;
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