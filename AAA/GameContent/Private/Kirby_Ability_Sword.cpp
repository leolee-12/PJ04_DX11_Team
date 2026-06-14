#include "Kirby_Ability_Sword.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Ability_Sword::CKirby_Ability_Sword()
{
}

HRESULT CKirby_Ability_Sword::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_ABILITY_TYPE CKirby_Ability_Sword::Get_AbilityType()
{
    return KIRBY_ABILITY_TYPE::SWORD;
}

void CKirby_Ability_Sword::Enter_Ability(CKirby* pKirby)
{

}

void CKirby_Ability_Sword::Update_Ability(CKirby* pKirby, _float fTimeDelta)
{
    CMovement_Child* pMovementCom = pKirby->Get_Movement();

    m_bEndAttack = true;
}

void CKirby_Ability_Sword::Exit_Ability(CKirby* pKirby)
{
}

_bool CKirby_Ability_Sword::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
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

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            pKirby->Add_MoveDir(pMoveCommand->Get_Dir());
            return true;
        }
    }

    return false;
}

void CKirby_Ability_Sword::Down_Attack(CKirby* pKirby)
{
    m_bIsFinished = false;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
}

void CKirby_Ability_Sword::Up_Attack(CKirby* pKirby)
{
    m_bIsFinished = true;
}

_bool CKirby_Ability_Sword::Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation)
{
    switch (eAttackLocation)
    {
        case KIRBY_ATTACK_LOCATION::GROUND:     return true;
        case KIRBY_ATTACK_LOCATION::AIR:        return true;
    }

    return false;
}

CKirby_Ability_Sword* CKirby_Ability_Sword::Create()
{
    CKirby_Ability_Sword* pInstance = new CKirby_Ability_Sword();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Sword::Free()
{
    __super::Free();
}