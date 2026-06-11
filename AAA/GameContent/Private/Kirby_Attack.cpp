#include "Kirby_Attack.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_Attack::CKirby_Attack()
{
}

HRESULT CKirby_Attack::Initialize()
{
    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Attack::Get_StateType()
{
    return KIRBY_STATE_TYPE::ATTACK;
}

void CKirby_Attack::Enter(CKirby* pKirby)
{
    pKirby->Get_KirbyAbility()->Enter_Ability(pKirby);
}

void CKirby_Attack::Update(CKirby* pKirby, const _float fTimeDelta)
{
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Update_Ability(pKirby, fTimeDelta);

    if (pAbility->IsEndAttack() == true)
    {
        Transition_Wait_OR_Run(pKirby);
    }
}

void CKirby_Attack::Exit(CKirby* pKirby)
{
    pKirby->Get_KirbyAbility()->Exit_Ability(pKirby);
}

_bool CKirby_Attack::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Attack Up
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsUp())
                return false;

            pKirby->Get_KirbyAbility()->Up_Attack(pKirby);
            return true;
        }
    }

    // 능력 Command 처리
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    if (pAbility->Handle_Command(pKirby, pCommand) == true)
        return true;

    return false;
}

CKirby_Attack* CKirby_Attack::Create()
{
    CKirby_Attack* pInstance = new CKirby_Attack();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Attack");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Attack::Free()
{
    __super::Free();
}
