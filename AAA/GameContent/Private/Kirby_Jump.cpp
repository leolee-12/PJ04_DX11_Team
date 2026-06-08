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
     const _float fSpeed = 5.f;
    if (m_bLeftRight == true)
        pAnimator->Play("JumpL", false, false, 0.1f, fSpeed);
    else
        pAnimator->Play("JumpR", false, false, 0.1f, fSpeed);

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
        CKirby_Body* pKirby_Body = pKirby->Get_Body();
        CAnimator* pAnimator = pKirby_Body->Get_Animator();

        _bool bIsGround = pMovementCom->Is_Grounded();

        if (m_eJumpType == JUMP_STATE::JUMP_STRAT && bIsGround == true)
        {
            pAnimator->Play("Landing", false, false, 0.05f, 1.f);

            pKirby_Body->Set_Eye(KIRBY_EYE_STATE::CLOSE);

            m_eJumpType = JUMP_STATE::LAND_START;
        }
        else if (m_eJumpType == JUMP_STATE::LAND_START && pAnimator->Is_Finished())
        {
            pKirby_Body->Set_Eye(KIRBY_EYE_STATE::IDLE);

            if (pKirby->Has_MoveDir() == true)
                pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
            else
                pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);

 /*           pAnimator->Play("LandingEnd", false, false, 0.05f, 2.f);
            m_eJumpType = JUMP_STATE::LAND_END;*/
        }
        //else if (m_eJumpType == JUMP_STATE::LAND_END && pAnimator->Is_Finished())
        //{
        //    if(pKirby->Has_MoveDir() == true)
        //        pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
        //    else
        //        pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);
        //}
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
