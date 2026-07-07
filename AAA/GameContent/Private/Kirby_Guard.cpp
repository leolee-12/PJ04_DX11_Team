#include "Kirby_Guard.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

#include "Kirby_Dodge.h"

CKirby_Guard::CKirby_Guard()
{
}

HRESULT CKirby_Guard::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Guard::Get_StateType()
{
    return KIRBY_STATE_TYPE::GUARD;
}

void CKirby_Guard::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);
    pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::GUARD);

    pKirby->Get_Body()->Set_KirbyEye(KIRBY_EYE_STATE::ANGRY);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_GroundFriction(s_fGuardGroundFriction);

    m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_Guard.wav", 0.5f);
}

void CKirby_Guard::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_Guard::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    pKirby->Get_Body()->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_GroundFriction(CKirby::s_fGroundFriction);
}

_bool CKirby_Guard::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
            pKirby->Change_State(KIRBY_STATE_TYPE::DODGE, DODGE_STATE_FLAG::DODGE_FRONT);
            break;
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
            pKirby->Change_State(KIRBY_STATE_TYPE::DODGE, DODGE_STATE_FLAG::DODGE_BACK);
            break;
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
            pKirby->Change_State(KIRBY_STATE_TYPE::DODGE, DODGE_STATE_FLAG::DODGE_LEFT);
            break;
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
            pKirby->Change_State(KIRBY_STATE_TYPE::DODGE, DODGE_STATE_FLAG::DODGE_RIGHT);
            break;

        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
            if (pAbility->Can_Attack(KIRBY_ATTACK_LOCATION::GROUND))
            {
                if (pCommand->IsDown())
                    pAbility->Enter_Attack_KeyDown(pKirby);
                else if (pCommand->IsPress())
                    pAbility->Enter_Attack_KeyPress(pKirby);
                else if (pCommand->IsUp())
                    pAbility->Enter_Attack_KeyUp(pKirby);
            }

            return true;
        }
        // Guard Up
        case KIRBY_COMMAND_TYPE::GUARD:
        {
            if (!pCommand->IsUp())
                return false;

            Transition_Fall_OR_Wait_OR_Run(pKirby);
            return true;
        }
    }

    return false;
}

void CKirby_Guard::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Apply_Knockback(tInfo.vAttackerPos, 50.f, 0.f);
}

CKirby_Guard* CKirby_Guard::Create()
{
    CKirby_Guard* pInstance = new CKirby_Guard();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Guard");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Guard::Free()
{
    __super::Free();
}
