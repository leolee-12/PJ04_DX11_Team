#include "Kirby_ControllableState.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_ControllableState::CKirby_ControllableState()
{
}

HRESULT CKirby_ControllableState::Initialize()
{
    if(FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

void CKirby_ControllableState::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);
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

            if (pKirby->Has_Deform())
            {
                if (pKirby->Can_Dump() == true)
                {
                    pKirby->Change_State(KIRBY_STATE_TYPE::DEFORM_DUMP);
                    pKirby->Reset_DumpCool();
                    return true;
                }
            }
            else
            {
                COPY_ABILITY_TYPE eAbilityType = pKirby->Get_KirbyAbility()->Get_AbilityType();
                if (eAbilityType == COPY_ABILITY_TYPE::NORMAL)
                    return true;

                if (pKirby->Can_Dump() == true)
                {
                    pKirby->Change_State(KIRBY_STATE_TYPE::ABILITY_DUMP);
                    pKirby->Reset_DumpCool();
                    return true;
                }
            }
    
            pKirby->Req_AbilityDumpCoolDecrease();

            return true;
        }
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (pKirby->IsTriggerDeformObj())
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
