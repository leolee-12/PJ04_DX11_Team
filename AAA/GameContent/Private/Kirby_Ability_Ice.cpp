#include "Kirby_Ability_Ice.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Ability_Ice::CKirby_Ability_Ice()
{
}

HRESULT CKirby_Ability_Ice::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"¾ÆÀÌ½º";

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Ice::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::ICE;
}

void CKirby_Ability_Ice::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = true;
}

void CKirby_Ability_Ice::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Ability_Ice::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Ability_Ice::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {

            return true;
        }
        // Jump Down
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            return true;
        }
    }

    return false;
}

_bool CKirby_Ability_Ice::Enter_Attack_KeyDown(CKirby* pKirby)
{
    return true;
}

_bool CKirby_Ability_Ice::Enter_Attack_KeyPress(CKirby* pKirby)
{
    return true;
}

_bool CKirby_Ability_Ice::Enter_Attack_KeyUp(CKirby* pKirby)
{
    return true;
}

CKirby_Ability_Ice* CKirby_Ability_Ice::Create()
{
    CKirby_Ability_Ice* pInstance = new CKirby_Ability_Ice();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Ice");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Ice::Free()
{
    __super::Free();
}
