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

void CKirby_Attack::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    pKirby->Get_ActiveAttackMode()->Enter_AttackState(pKirby, iFlag);
}

void CKirby_Attack::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_AttackMode* pAttackMode = pKirby->Get_ActiveAttackMode();
    pAttackMode->Update_AttackState(pKirby, fTimeDelta);

    if (pAttackMode->Get_ReqEndAttackState() == true)
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

    pKirby->Get_ActiveAttackMode()->Exit_AttackState(pKirby);
}

_bool CKirby_Attack::Ignore_TimeScale(CKirby* pKirby)
{
    CKirby_AttackMode* pAttackMode = pKirby->Get_ActiveAttackMode();

    return pAttackMode && pAttackMode->Ignore_TimeScale();
}

void CKirby_Attack::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
        pKirby->Get_ActiveAttackMode()->On_Damaged_KirbyState(pKirby, tInfo);
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

    if (pKirby->Get_ActiveAttackMode()->Handle_Command(pKirby, pCommand))
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
