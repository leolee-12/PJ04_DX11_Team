#include "Kirby_Wait.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

CKirby_Wait::CKirby_Wait()
{
}

HRESULT CKirby_Wait::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Wait::Get_StateType()
{
    return KIRBY_STATE_TYPE::WAIT;
}

void CKirby_Wait::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    if (pKirby->Has_Deform())
        pKirby->Get_KirbyDeform()->Play_DeformAni(pKirby, DEFORM_ANI::WAIT);
    else
        pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::WAIT);
}

void CKirby_Wait::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    // Fall
    if (Try_Transition_Fall(pKirby) == true)
        return;
}

void CKirby_Wait::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    pKirby->Get_CurrentDeformModel()->Stop_SoundHandle();
}

_bool CKirby_Wait::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

CKirby_Wait* CKirby_Wait::Create()
{
    CKirby_Wait* pInstance = new CKirby_Wait();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Wait");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Wait::Free()
{
    __super::Free();
}
