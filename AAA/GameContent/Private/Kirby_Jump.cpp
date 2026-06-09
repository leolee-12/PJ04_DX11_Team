#include "Kirby_Jump.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"

 _bool CKirby_Jump::m_bLeft = false;


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
     const _float fSpeed = 5.f;
    if (m_bLeft == true)
        pAnimator->Play("JumpL", false, false, 0.1f, fSpeed);
    else
        pAnimator->Play("JumpR", false, false, 0.1f, fSpeed);

    m_bFirstFrameSkip = false;
    m_eJumpType = JUMP_STATE::JUMP_STRAT;
}

void CKirby_Jump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    if (m_bFirstFrameSkip == false)
    {
        m_bFirstFrameSkip = true;
        return;
    }

    CMovement* pMovementCom = pKirby->Get_Movement();

    _float fYVelocity = pMovementCom->Get_VerticalVelocity();
    _bool bIsGround = pMovementCom->Is_Grounded();

    // Fall
    if (fYVelocity <= 0.005f)
    {
        pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
        if (rand() % 2 == 0)
        {
            CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

            const _float fSpeed = 2.2f;

            if (m_bLeft == true)
                pAnimator->Play("JumpEndL", false, false, 0.0f, fSpeed);
            else
                pAnimator->Play("JumpEndR", false, false, 0.0f, fSpeed);
        }
    }      
}

void CKirby_Jump::Exit(CKirby* pKirby)
{
    m_bFirstFrameSkip = false;
    m_bLeft = !m_bLeft;
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
