#include "Kirby_Attack.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"
#include "Kirby_Deform.h"

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

    if (pKirby->Has_Deform())
        pKirby->Get_KirbyDeform()->Enter_DeformState(pKirby);
    else
        pKirby->Get_KirbyAbility()->Enter_AbilityState(pKirby);
}

void CKirby_Attack::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    _bool bReqEndAttackState{};

    if (pKirby->Has_Deform())
    {
        CKirby_Deform* pDeform = pKirby->Get_KirbyDeform();
        pDeform->Update_DeformState(pKirby, fTimeDelta);
        bReqEndAttackState = pDeform->ReqEndAttackState();
    }
    else
    {
        CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
        pAbility->Update_AbilityState(pKirby, fTimeDelta);
        bReqEndAttackState = pAbility->ReqEndAttackState();
    }

    if (bReqEndAttackState == true)
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

    if (pKirby->Has_Deform())
        pKirby->Get_KirbyDeform()->Exit_DeformState(pKirby);
    else
        pKirby->Get_KirbyAbility()->Exit_AbilityState(pKirby);
}

void CKirby_Attack::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    if (pKirby->Has_Deform())
        pKirby->Get_KirbyDeform()->On_Damaged_KirbyState(pKirby, tInfo);
    else
        pKirby->Get_KirbyAbility()->On_Damaged_KirbyState(pKirby, tInfo);
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

            if (pKirby->Has_Deform())
                return true;

            m_bGuardReserved = true;
            return true;
        }
    }

    if (pKirby->Has_Deform())
    {
        CKirby_Deform* pDeform = pKirby->Get_KirbyDeform();
        if (pDeform->Handle_Command(pKirby, pCommand) == true)
            return true;
    }
    else
    {
        CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
        if (pAbility->Handle_Command(pKirby, pCommand) == true)
            return true;
    }

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
