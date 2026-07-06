#include "Kirby_Run.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

CKirby_Run::CKirby_Run()
{
}

HRESULT CKirby_Run::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Run::Get_StateType()
{
    return KIRBY_STATE_TYPE::RUN;
}

void CKirby_Run::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    if (pKirby->Has_Deform())
        pKirby->Get_KirbyDeform()->Play_DeformAni(pKirby, DEFORM_ANI::RUN);
    else
        pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::RUN);
}

void CKirby_Run::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    // Fall
    if (Try_Transition_Fall(pKirby) == true)
        return;

    // Wait
    if (pKirby->Has_MoveDir() == false)
        pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);
}

void CKirby_Run::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    pKirby->Get_CurrentDeformModel()->Stop_SoundHandle();
}

_bool CKirby_Run::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        {
            if (pKirby->IsLadder() && pCommand->IsPress())
            {
                pKirby->Change_State(KIRBY_STATE_TYPE::LADDER);
                return true;
            }
        }

        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            Handle_MoveCommand(pKirby, pCommand);
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

CKirby_Run* CKirby_Run::Create()
{
    CKirby_Run* pInstance = new CKirby_Run();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Run");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Run::Free()
{
    __super::Free();
}