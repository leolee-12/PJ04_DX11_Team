#include "Kirby_Hovering.h"

#include "GameInstance.h"

#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Fall.h"

CKirby_Hovering::CKirby_Hovering()
{
}

HRESULT CKirby_Hovering::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Hovering::Get_StateType()
{
    return KIRBY_STATE_TYPE::HOVERING;
}

void CKirby_Hovering::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::FLIGHT_START);

    // State
    m_eHoveringState = HOVERING_STATE::FLIGHT_START;

    // Mesh
    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_KirbyBody(KIRBY_BODY_STATE::STUFFED);

    // Movement
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    pMovementCom->Set_GravityScale(0.6f);
    pMovementCom->Set_MaxFallVelocity(s_fHoveringMaxFallVelocity);
    pMovementCom->Set_LinearDrag(s_fHoveringLinearDrag);
    pMovementCom->Set_MaxHorizontalSpeed(s_fHoveringMaxHorizontalSpeed);

    m_eCurMoveState = HOVERING_MOVE_STATE::FALL;
    m_bPlayFlightAni = false;
}

void CKirby_Hovering::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    if (Update_HoveringStateMachine(pKirby, fTimeDelta) == true)
        return;

    if(m_eHoveringState == HOVERING_STATE::FLIGHT_LOOP)
        Update_LoopState(pKirby, fTimeDelta);
}

void CKirby_Hovering::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    // Mesh
    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_KirbyBody(KIRBY_BODY_STATE::NORMAL);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    Reset_Movement(pMovement);
}

_bool CKirby_Hovering::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    CMovement_Child* pMovementCom = pKirby->Get_Movement();

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        {
            if (!pCommand->IsPress())
                return false;

            if (Try_Transition_Ladder_CommandUp(pKirby))
                return true;

            if (m_bMoveLock)
                return true;

            Handle_MoveCommand(pKirby, pCommand);
            return true;
        }

        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_bMoveLock)
                return true;

            Handle_MoveCommand(pKirby, pCommand);
            return true;
        }
        // Jump Press
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsPress())
                return false;

            if(m_eHoveringState == HOVERING_STATE::FLIGHT_LOOP)
            {
                if (m_eCurMoveState == HOVERING_MOVE_STATE::FALL)
                {
                    m_bPlayFlightAni = true;
                    m_eCurMoveState = HOVERING_MOVE_STATE::JUMP;
                }
                else if(m_eCurMoveState == HOVERING_MOVE_STATE::JUMP)
                {
                    if (pAnimator->Is_Finished() == true)                              
                        m_bPlayFlightAni = true;                    
                }

                if (m_bPlayFlightAni == true)
                {
                    pMovementCom->Set_VelocityY(0.f);
                    //pMovementCom->Add_Velocity(XMVectorSet(0.f, 10.f, 0.f, 0.f));
                    pMovementCom->Add_Velocity(XMVectorSet(0.f, 15.f, 0.f, 0.f));
                }
            }
            return true;
        }
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eHoveringState != HOVERING_STATE::FLIGHT_LOOP)
                return true;

            m_eHoveringState = HOVERING_STATE::FLIGHT_END;
            CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::FLIGHT_LANDING);

            m_bMoveLock = true;

            return true;
        }
    }

    return false;
}

_bool CKirby_Hovering::Update_HoveringStateMachine(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    CMovement_Child* pMovement = pKirby->Get_Movement();
    
    switch (m_eHoveringState)
    {
        case HOVERING_STATE::FLIGHT_START:
        {
            if (pAnimator->Is_Finished() == true)
                m_eHoveringState = HOVERING_STATE::FLIGHT_LOOP;

            Check_Landing(pKirby, pAnimator, pMovement);
  
            return true;
        }

        case HOVERING_STATE::FLIGHT_LOOP:
        {
            return Check_Landing(pKirby, pAnimator, pMovement);
        }

        case HOVERING_STATE::FLIGHT_END:
        {
            // FlightLanding이 끝나면
            if (pAnimator->Is_Finished() == true)
            {
                m_eHoveringState = HOVERING_STATE::SPIT_AIR;
                CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
                pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::AIR_BALL);

                pBody->Set_KirbyBody(KIRBY_BODY_STATE::INHALE);

                m_bMoveLock = true;
            }

            return true;
        }

        case HOVERING_STATE::SPIT_AIR:
        {
            if (pAnimator->Is_Finished() == true)
            {
                _bool bIsGround = pMovement->Is_Grounded();
                if (bIsGround == false)
                    pKirby->Change_State(KIRBY_STATE_TYPE::FALL, FALL_STATE_FLAG::FALL_DIRECT);
                else
                    Transition_Wait_OR_Run(pKirby);
                
                pBody->Set_KirbyBody(KIRBY_BODY_STATE::NORMAL);

                Reset_Movement(pMovement);

                m_bMoveLock = false;
            }

            return true;
        }
    }

    return false;
}

_bool CKirby_Hovering::Check_Landing(CKirby* pKirby, CAnimator* pAnimator, CMovement_Child* pMovement)
{
    _bool bIsGround = pMovement->Is_Grounded();

    if (bIsGround == true)
    {
        m_eHoveringState = HOVERING_STATE::FLIGHT_END;
        CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
        pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::FLIGHT_LANDING);

        Reset_Movement(pMovement);

        m_bMoveLock = true;

        return true;
    }

    return false;
}

void CKirby_Hovering::Update_LoopState(CKirby* pKirby, _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();

    if (m_bPlayFlightAni == true)
    {
        pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::FLIGHT);

        m_bPlayFlightAni = false;
    }
    else
    {
        if(pAnimator->Is_Finished())
        {
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::FLIGHT_FALL);
            m_eCurMoveState = HOVERING_MOVE_STATE::FALL;
        }
    }
}

void CKirby_Hovering::Reset_Movement(CMovement_Child* pMovement)
{
    pMovement->Set_GravityScale(1.f);
    pMovement->Set_MaxFallVelocity(CKirby::s_fMaxFallVelocity);
    pMovement->Set_LinearDrag(CKirby::s_fLinearDrag);
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
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
