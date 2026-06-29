#include "Kirby_ControllableState.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_ControllableState::CKirby_ControllableState()
{
}

HRESULT CKirby_ControllableState::Initialize()
{
    if(FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

void CKirby_ControllableState::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);
}

void CKirby_ControllableState::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_ControllableState::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_ControllableState::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Dump
        case KIRBY_COMMAND_TYPE::DUMP:
        {
            if (!pCommand->IsPress())
                return false;

            COPY_ABILITY_TYPE eAbilityType = pKirby->Get_KirbyAbility()->Get_AbilityType();
            if (eAbilityType == COPY_ABILITY_TYPE::NORMAL)
                return true;

            if (pKirby->Can_AbilityDump() == true)
            {
                pKirby->Change_State(KIRBY_STATE_TYPE::ABILITY_DUMP);
                pKirby->Reset_AbilityDumpCool();
                return true;
            }

            pKirby->Req_AbilityDumpCoolDecrease();

            return true;
        }

        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (GetAsyncKeyState('F') & 0x8000)
            {
                if (!pCommand->IsDown())
                    return false;

                if (pKirby->Get_KirbyDeform() != nullptr)
                    return false;

                pKirby->Change_State(KIRBY_STATE_TYPE::GET_DEFORM);

                return true;
            }

            return false;
        }
    }

    return false;
}

void CKirby_ControllableState::Free()
{
    __super::Free();
}
