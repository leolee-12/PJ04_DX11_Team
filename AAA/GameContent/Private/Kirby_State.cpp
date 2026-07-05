#include "Kirby_State.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_State::CKirby_State()
{
}

HRESULT CKirby_State::Initialize()
{
    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    return S_OK;
}

void CKirby_State::Enter(CKirby* pKirby)
{
}

void CKirby_State::Update(CKirby* pKirby, const _float fTimeDelta)
{
    pKirby->Update_AbilityDumpCool(fTimeDelta);
}

void CKirby_State::Exit(CKirby* pKirby)
{
}

void CKirby_State::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Apply_Knockback(tInfo.vAttackerPos, 180.f, 9.f);  // Test

    pKirby->Add_HP(-tInfo.fDamage);
    pKirby->Start_DamageInvincibility();

    pKirby->Change_State(KIRBY_STATE_TYPE::DAMAGED);
}

_bool CKirby_State::Handle_MoveCommand(CKirby* pKirby, CKirby_Command* pCommand)
{
    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    if (eCommandType == KIRBY_COMMAND_TYPE::MOVE_TOP || eCommandType == KIRBY_COMMAND_TYPE::MOVE_DOWN ||
        eCommandType == KIRBY_COMMAND_TYPE::MOVE_LEFT || eCommandType == KIRBY_COMMAND_TYPE::MOVE_RIGHT)
    {
        if (!pCommand->IsPress())
            return false;

        Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
        pKirby->Add_MoveDir(pMoveCommand->Get_Dir());

        return true;
    }

    return false;
}

_bool CKirby_State::Try_Transition_Fall(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    _float fYVelocity = pMovement->Get_VerticalVelocity();

    _bool bIsGround = pMovement->Is_Grounded();
    if (bIsGround == false && fYVelocity <= CKirby::s_fFallVelocityY)
    {  
        pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
        return true;
    }

    return false;
}

_bool CKirby_State::Transition_Wait_OR_Run(CKirby* pKirby)
{
    if (pKirby->Has_MoveDir() == true)
        pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
    else
        pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);

    return true;
}

_bool CKirby_State::Transition_Fall_OR_Wait_OR_Run(CKirby* pKirby)
{
    if (Try_Transition_Fall(pKirby))
        return true;

    return Transition_Wait_OR_Run(pKirby);
}

_bool CKirby_State::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    // 전역 처리

    return false;
}

void CKirby_State::Request_GrabState(CKirby* pKirby, GRAB_TYPE eType)
{
    switch (eType)
    {
        case GRAB_TYPE::GORILLA_SCENE:
        {
            pKirby->Change_State(KIRBY_STATE_TYPE::CUTSCENE_GRABBED);
            break;
        }
        case GRAB_TYPE::GORILLA_COMBAT:
        {
            pKirby->Change_State(KIRBY_STATE_TYPE::QTE_GRABBED);
            break;
        }
    }
}

void CKirby_State::Request_ReleaseGrabState(CKirby* pKirby, GRAB_TYPE eType)
{
}

void CKirby_State::Free()
{
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}
