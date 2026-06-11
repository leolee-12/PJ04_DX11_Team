#include "Kirby_Hovering.h"

#include "GameInstance.h"

#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_Hovering::CKirby_Hovering()
{
}

HRESULT CKirby_Hovering::Initialize()
{
    m_fMaxJumpTime = 0.4;
    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Hovering::Get_StateType()
{
    return KIRBY_STATE_TYPE::HOVERING;
}

void CKirby_Hovering::Enter(CKirby* pKirby)
{
    m_bCanJump = true;

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    // Ani
    pAnimator->Play("FlightStart", false, false, 0.1f, 2.25f);

    // State
    m_eHoveringState = HOVERING_STATE::FLIGHT_START;

    // Mesh
    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Body(KIRBY_BODY_STATE::STUFFED);

    // Speed
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    pMovementCom->Set_GravityScale(0.6f);
    pMovementCom->Set_MaxFallVelocity(-1.5f);
}

void CKirby_Hovering::Update(CKirby* pKirby, const _float fTimeDelta)
{
    Update_CoolTimer(fTimeDelta);

    if (Update_State(pKirby, fTimeDelta) == true)
        return;

    Update_MoveState(pKirby, fTimeDelta);
    Enter_Ani(pKirby);
}

void CKirby_Hovering::Exit(CKirby* pKirby)
{
    // Mesh
    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Body(KIRBY_BODY_STATE::NORMAL);
}

_bool CKirby_Hovering::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

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

            Handle_MoveCommand(pKirby, pCommand);
            return true;
        }
        // Jump Press
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_bCanJump == true)
            {
                CMovement_Child* pMovementCom = pKirby->Get_Movement();
                pMovementCom->Set_VelocityY(0.f);
                const _float fSpeed = 8.f;
                pMovementCom->Add_Velocity(XMVectorSet(0.f, fSpeed, 0.f, 0.f));
                Reset_CoolTimer();
            }
                return true;
        }
    }

    return false;
}

_bool CKirby_Hovering::Update_State(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    _bool bEarlyReturn = { false };

    switch (m_eHoveringState)
    {
        case HOVERING_STATE::FLIGHT_START:
        {
            if (pAnimator->Is_Finished() == true)
                m_eHoveringState = HOVERING_STATE::FLIGHT_LOOP;

            bEarlyReturn = true;
  
            // break x ÀÇµµ
        }

        case HOVERING_STATE::FLIGHT_LOOP:
        {
            CMovement_Child* pMovementCom = pKirby->Get_Movement();
            _bool bIsGround = pMovementCom->Is_Grounded();

            if (bIsGround == true)
            {
                // ³¡
                m_eHoveringState = HOVERING_STATE::FLIGHT_END;
                pAnimator->Play("FlightLanding", false, false, 0.1f, 2.f);

                pMovementCom->Set_GravityScale(1.f);
                pMovementCom->Set_MaxFallVelocity(-15.f);

                bEarlyReturn = true;
            }
            break;
        }

        case HOVERING_STATE::FLIGHT_END:
        {
            if (pAnimator->Is_Finished() == true)
            {
                m_eHoveringState = HOVERING_STATE::SPITAIR;
                pAnimator->Play("AirBall", false, false, 0.0f, 3.5f);

                pBody->Set_Body(KIRBY_BODY_STATE::INHALE);

                bEarlyReturn = true;
            }

            bEarlyReturn = true;
            break;
        }

        case HOVERING_STATE::SPITAIR:
        {
            if (pAnimator->Is_Finished() == true)
            {
                Transition_Wait_OR_Run(pKirby);
                pBody->Set_Body(KIRBY_BODY_STATE::NORMAL);
            }

            bEarlyReturn = true;
            break;
        }
    }

    return bEarlyReturn;
}

void CKirby_Hovering::Enter_Ani(CKirby* pKirby)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    if (m_eCurMoveState != m_ePreMoveState)
    {
        switch (m_eCurMoveState)
        {
        case HOVERING_MOVE_STATE::FALL:
            pAnimator->Play("FlightFall", true, false, 0.1f, 2.f);
            break;
        case HOVERING_MOVE_STATE::MOVE:
            pAnimator->Play("Flight", true, false, 0.1f, 2.25f);
            break;
        }

        m_ePreMoveState = m_eCurMoveState;
    }
}

void CKirby_Hovering::Update_MoveState(CKirby* pKirby, _float fTimeDelta)
{
    // Move State

    _bool m_bIsMoving = pKirby->Has_MoveDir();
    if (m_bIsMoving == false)
        m_eCurMoveState = HOVERING_MOVE_STATE::FALL;
    else if (m_bIsMoving == true)
        m_eCurMoveState = HOVERING_MOVE_STATE::MOVE;
}

void CKirby_Hovering::Update_CoolTimer(_float fTimeDelta)
{
    if (m_fAccJumpTime < m_fMaxJumpTime)
    {
        m_fAccJumpTime += fTimeDelta;

        if (m_fAccJumpTime >= m_fMaxJumpTime)
            m_bCanJump = true;
    }
}

void CKirby_Hovering::Reset_CoolTimer()
{
    m_bCanJump = false;
    m_fAccJumpTime = 0.f;
}

CKirby_Hovering* CKirby_Hovering::Create()
{
    CKirby_Hovering* pInstance = new CKirby_Hovering();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Hovering");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Hovering::Free()
{
    __super::Free();
}
