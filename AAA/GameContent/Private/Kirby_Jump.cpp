#include "Kirby_Jump.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"

 _bool CKirby_Jump::m_bLeftRight = false;


CKirby_Jump::CKirby_Jump()
{
}

HRESULT CKirby_Jump::Initialize()
{

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Jump::Get_StateType()
{
    return KIRBY_STATE_TYPE::JUMP;
}

void CKirby_Jump::Enter(CKirby* pKirby)
{
    CMovement* pMovementCom = static_cast<CMovement*>(pKirby->Get_Component<CMovement>(TEXT("Com_Movement")));
    pMovementCom->Jump();


    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    if (m_bLeftRight == true)
        pAnimator->Play("JumpL", false);
    else
        pAnimator->Play("JumpR", false);

    m_bFirstFrameSkip = false;
    m_eJumpType = JUMP_STATE::JUMP_STRAT;
}

void CKirby_Jump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    CMovement* pMovementCom = static_cast<CMovement*>(pKirby->Get_Component<CMovement>(TEXT("Com_Movement")));

    if (m_bFirstFrameSkip == false)
    {
        m_bFirstFrameSkip = true;
        return;
    }

    if (m_bFirstFrameSkip == true)
    {
        CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

        _bool bIsGround = pMovementCom->Is_Grounded();

        if (m_eJumpType == JUMP_STATE::JUMP_STRAT && bIsGround == true)
        {
            pAnimator->Play("Landing", false);
            m_eJumpType = JUMP_STATE::LAND_START;
        }
        else if (m_eJumpType == JUMP_STATE::LAND_START && pAnimator->Is_Finished())
        {
            pAnimator->Play("LandingEnd", false);
            m_eJumpType = JUMP_STATE::LAND_END;
        }
        else if (m_eJumpType == JUMP_STATE::LAND_END && pAnimator->Is_Finished())
        {
             pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);
        }
    }
}

void CKirby_Jump::Exit(CKirby* pKirby)
{
    m_bFirstFrameSkip = false;
    m_bLeftRight = !m_bLeftRight;
}

_bool CKirby_Jump::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
            Handle_MoveCommand(pKirby, pCommand);
            return true;
    }

    return false;
}

CKirby_Jump* CKirby_Jump::Create()
{
    CKirby_Jump* pInstance = new CKirby_Jump();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Jump");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Jump::Free()
{
    __super::Free();
}
