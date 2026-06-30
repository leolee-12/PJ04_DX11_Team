#include "Kirby_Attack.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_Attack::CKirby_Attack()
{
}

HRESULT CKirby_Attack::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Attack::Get_StateType()
{
    return KIRBY_STATE_TYPE::ATTACK;
}

void CKirby_Attack::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // 현재 능력에게 Enter 전달
    pKirby->Get_KirbyAbility()->Enter_AbilityState(pKirby);
}

void CKirby_Attack::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    // 현재 능력에게 Update 전달
    ABILITY_UPDATE_RESULT eUpdateResult= pAbility->Update_AbilityState(pKirby, fTimeDelta);

    if (eUpdateResult == ABILITY_UPDATE_RESULT::ABILITY_CHANGED)
        return;

    // Ability가 Attack State가 끝났다고 하면 State 전환
    if (pAbility->ReqEndAttackState() == true)
    {
        CMovement_Child* pMovement = pKirby->Get_Movement();
        
        if (m_bGuardReserved && pMovement->Is_Grounded())
            pKirby->Change_State(KIRBY_STATE_TYPE::GUARD);
        else
            Transition_Fall_OR_Wait_OR_Run(pKirby);
    }

    m_bGuardReserved = false;
}

void CKirby_Attack::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    // 현재 능력에게 Exit 전달
    pKirby->Get_KirbyAbility()->Exit_AbilityState(pKirby);
}

_bool CKirby_Attack::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Guard
        case KIRBY_COMMAND_TYPE::GUARD:
        {
            if (!pCommand->IsPress())
                return false;

            m_bGuardReserved = true;
            return true;
        }
    }

    // Ability가 Command 처리
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
