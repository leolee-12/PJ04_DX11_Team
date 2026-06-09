#include "Kirby_Fall.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_Fall::CKirby_Fall()
{
}

HRESULT CKirby_Fall::Initialize()
{

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Fall::Get_StateType()
{
    return KIRBY_STATE_TYPE::FALL;
}

void CKirby_Fall::Enter(CKirby* pKirby)
{
    m_eFallingState = FALL_STATE::FALLING;
}

void CKirby_Fall::Update(CKirby* pKirby, const _float fTimeDelta)
{
    CMovement* pMovementCom = pKirby->Get_Movement();

    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();

    // 착지 후 애니메이션
    if (m_eFallingState == FALL_STATE::FALLING &&
        pMovementCom->Is_Grounded() == true)
    {
        m_eFallingState = FALL_STATE::LAND_START;
        pAnimator->Play("Landing", false, false, 0.05f, 1.f);
        pKirby_Body->Set_Eye(KIRBY_EYE_STATE::CLOSE);
    }
    // 애니메이션 끝나면 Wait or Run
    else if (m_eFallingState == FALL_STATE::LAND_START &&
        pAnimator->Is_Finished())
    {
        Transition_Wait_OR_Run(pKirby);
    }
}

void CKirby_Fall::Exit(CKirby* pKirby)
{
    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();
    pKirby_Body->Set_Eye(KIRBY_EYE_STATE::IDLE);
}

_bool CKirby_Fall::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

 /*       case KIRBY_COMMAND_TYPE::JUMP:
        {
            CMovement* pMovementCom = pKirby->Get_Movement();
            _float fYVelocity = pMovementCom->Get_VerticalVelocity();

            if (fYVelocity <= 0.15f)
            {
                pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
            }
            return true;
        }*/
    }

    return false;
}

CKirby_Fall* CKirby_Fall::Create()
{
    CKirby_Fall* pInstance = new CKirby_Fall();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Fall");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Fall::Free()
{
    __super::Free();
}