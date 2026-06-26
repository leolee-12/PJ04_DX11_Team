#include "Kirby_Guard.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_Guard::CKirby_Guard()
{
}

HRESULT CKirby_Guard::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Guard::Get_StateType()
{
    return KIRBY_STATE_TYPE::GUARD;
}

void CKirby_Guard::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);
    pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::GUARD);

    pKirby->Get_Body()->Set_Eye(KIRBY_EYE_STATE::ANGRY);
}

void CKirby_Guard::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_Guard::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    pKirby->Get_Body()->Set_Eye(KIRBY_EYE_STATE::IDLE);
}

_bool CKirby_Guard::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        //// Move Press
        //case KIRBY_COMMAND_TYPE::MOVE_TOP:
        //case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        //case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        //case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        //{
        //    if (!pCommand->IsPress())
        //        return false;

        //    Handle_MoveCommand(pKirby, pCommand);
        //    pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
        //    return true;
        //}
        //// Jump Down
        //case KIRBY_COMMAND_TYPE::JUMP:
        //{
        //    if (!pCommand->IsDown())
        //        return false;

        //    pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
        //    return true;
        //}
        //// Attack
        //case KIRBY_COMMAND_TYPE::ATTACK:
        //{
        //    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
        //    if (pAbility->Can_Attack(KIRBY_ATTACK_LOCATION::GROUND))
        //    {
        //        if (pCommand->IsDown())
        //            pAbility->Enter_Attack_KeyDown(pKirby);
        //        else if (pCommand->IsPress())
        //            pAbility->Enter_Attack_KeyPress(pKirby);
        //        else if (pCommand->IsUp())
        //            pAbility->Enter_Attack_KeyUp(pKirby);
        //    }

        //    return true;
        //}
        // Guard
        case KIRBY_COMMAND_TYPE::GUARD:
        {
            if (!pCommand->IsUp())
                return false;

            Transition_Fall_OR_Wait_OR_Run(pKirby);
            return true;
        }
    }

    return false;
}

CKirby_Guard* CKirby_Guard::Create()
{
    CKirby_Guard* pInstance = new CKirby_Guard();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Guard");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Guard::Free()
{
    __super::Free();
}
