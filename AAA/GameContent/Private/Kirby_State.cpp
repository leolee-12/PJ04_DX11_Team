#include "Kirby_State.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

#include "LevelDesign_Ladder.h"

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

void CKirby_State::Enter(CKirby* pKirby, _int iFlag)
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

    m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_DamageNormal.wav", 0.5f);

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

_bool CKirby_State::Try_Transition_Ladder_CommandUp(CKirby* pKirby)
{
    // 있으면 충돌
    CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
    if (pLadder == nullptr)
        return false;

    CTransform* pTransform = pKirby->Get_Transform();
    _vector vPos = pTransform->Get_State(STATE::POSITION);

    _int iCellIndex = pLadder->Get_NearestCellIndex(vPos);
    if (pLadder->Is_TopCell(iCellIndex))
        return false;

    pKirby->Change_State(KIRBY_STATE_TYPE::LADDER);
    return true;
}

_bool CKirby_State::Try_Transition_Ladder_CommandDown(CKirby* pKirby)
{
    // 있으면 충돌
    CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
    if (pLadder == nullptr)
        return false;

    CTransform* pTransform = pKirby->Get_Transform();
    _vector vPos = pTransform->Get_State(STATE::POSITION);

    _int iCellIndex = pLadder->Get_NearestCellIndex(vPos);
    if (pLadder->Is_BottomCell(iCellIndex))
        return false;

    pKirby->Change_State(KIRBY_STATE_TYPE::LADDER);
    return true;
}

_bool CKirby_State::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    // 전역 처리

    return false;
}

void CKirby_State::Request_Attach(CKirby* pKirby, KIRBY_ATTACHMENT_CONTEXT eType)
{
    switch (eType)
    {
        case KIRBY_ATTACHMENT_CONTEXT::GORILLA_SCENE:
        {
            pKirby->Change_State(KIRBY_STATE_TYPE::CUTSCENE_GRABBED);
            break;
        }
        case KIRBY_ATTACHMENT_CONTEXT::GORILLA_COMBAT:
        {
            pKirby->Change_State(KIRBY_STATE_TYPE::QTE_GRABBED);
            break;
        }
        case KIRBY_ATTACHMENT_CONTEXT::DEFORM_CAR_GET_FIRST:
        {
            pKirby->Change_State(KIRBY_STATE_TYPE::CAR_FIRST_BREAK_WALL);
            break;
        }
    }
}

void CKirby_State::Request_Attach_End(CKirby* pKirby, KIRBY_ATTACHMENT_END_REASON eType)
{
}

void CKirby_State::Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc)
{
}

void CKirby_State::Request_PositionEndSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc)
{
}

void CKirby_State::Request_StageClear(CKirby* pKirby, const CUTSCENE_STAGECLEAR* pDesc)
{
}

void CKirby_State::Free()
{
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}
