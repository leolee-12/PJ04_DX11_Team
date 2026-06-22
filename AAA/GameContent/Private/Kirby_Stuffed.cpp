#include "Kirby_Stuffed.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_Stuffed::CKirby_Stuffed()
{
}

HRESULT CKirby_Stuffed::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_eCurStuffedState = STUFFED_STATE::STUFFED_END;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Stuffed::Get_StateType()
{
    return KIRBY_STATE_TYPE::FULL;
}

void CKirby_Stuffed::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // State
    m_eCurStuffedState = STUFFED_START;

    // Animation
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::STUFFED_START));

    // Body
    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Body(KIRBY_BODY_STATE::STUFFED);
}

void CKirby_Stuffed::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator(); 

    Update_State(pKirby);
    Update_Mouth(pKirby);
}

void CKirby_Stuffed::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    // Body
    CKirby_Body* pBody = pKirby->Get_Body();
    pKirby->Get_Body()->Set_Body(KIRBY_BODY_STATE::NORMAL);

    // State
    m_eCurStuffedState = STUFFED_STATE::STUFFED_END;
}

_bool CKirby_Stuffed::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

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

        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eCurStuffedState == STUFFED_STATE::STUFFED_SPIT)
                return true;

            CMovement_Child* pMovement = pKirby->Get_Movement();
            if (pMovement->Try_Jump())
                Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_JUMP);

            return true;
        }


        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eCurStuffedState == STUFFED_STATE::STUFFED_SPIT)
                return true;

            Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_SPIT);
            pKirby->Spit_Inhalable();
            return true;
        }
    }

    return false;
}

void CKirby_Stuffed::Update_State(CKirby* pKirby)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    switch (m_eCurStuffedState)
    {
        case STUFFED_STATE::STUFFED_START:
        {
            if (pAnimator->Is_Finished())
            {
                m_eCurStuffedState = STUFFED_STATE::STUFFED_WAIT;
                pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::STUFFED_WAIT));
            }

                break;
        }

        case STUFFED_STATE::STUFFED_WAIT:
        case STUFFED_STATE::STUFFED_RUN:
        case STUFFED_STATE::STUFFED_JUMP:
        case STUFFED_STATE::STUFFED_FALL:
        {
            Update_MoveState(pKirby);
            return;
        }

        case STUFFED_STATE::STUFFED_LANDING:
        {
            if (pAnimator->Is_Finished())
            {
                if (pKirby->Has_MoveDir())
                    Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_RUN);
                else
                    Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_WAIT);
            }

            return;
        }

        case STUFFED_STATE::STUFFED_SPIT:
        {
            if (pAnimator->Is_Finished())
            {
                CMovement_Child* pMovement = pKirby->Get_Movement();

                _float fYVelocity = pMovement->Get_VerticalVelocity();
                _bool bIsGround = pMovement->Is_Grounded();

                if (bIsGround == false)
                    pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
                else
                    Transition_Wait_OR_Run(pKirby);
            }
            return;
        }
    }
}

void CKirby_Stuffed::Update_MoveState(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    _float fYVelocity = pMovement->Get_VerticalVelocity();
    _bool bIsGround = pMovement->Is_Grounded();

    if (m_eCurStuffedState == STUFFED_STATE::STUFFED_JUMP)
    {
        if (fYVelocity <= 0.f)
            Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_FALL);

        return;
    }

    if (m_eCurStuffedState == STUFFED_STATE::STUFFED_FALL)
    {
        if (bIsGround == true)
            Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_LANDING);

        return;
    }

    if (bIsGround == false)
    {
        Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_FALL);
        return;
    }

    if (pKirby->Has_MoveDir())
        Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_RUN);
    else
        Change_StuffedState(pKirby, STUFFED_STATE::STUFFED_WAIT);
}

void CKirby_Stuffed::Change_StuffedState(CKirby* pKirby, STUFFED_STATE eNextState)
{
    if (m_eCurStuffedState == eNextState)
        return;

    m_eCurStuffedState = eNextState;

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    Enter_Animation(pKirby, pAnimator);
}

void CKirby_Stuffed::Enter_Animation(CKirby* pKirby, CAnimator* pAnimator)
{
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();

    switch (m_eCurStuffedState)
    {
        case STUFFED_STATE::STUFFED_START:
            pAnimator->Play(pAbility->Get_AniInfo(ABILITY_ANI::STUFFED_START));
            break;

        case STUFFED_STATE::STUFFED_WAIT:
            pAnimator->Play(pAbility->Get_AniInfo(ABILITY_ANI::STUFFED_WAIT));
            break;

        case STUFFED_STATE::STUFFED_RUN:
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::STUFFED_RUN);
            break;

        case STUFFED_STATE::STUFFED_JUMP:
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::STUFFED_JUMP);
            break;

        case STUFFED_STATE::STUFFED_FALL:
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::STUFFED_FALL);
            break;

        case STUFFED_STATE::STUFFED_LANDING:
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::STUFFED_LANDING);
            break;

        case STUFFED_STATE::STUFFED_SPIT:
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::STUFFED_SPIT);
            break;
    }
}

void CKirby_Stuffed::Update_Mouth(CKirby* pKirby)
{

    switch (m_eCurStuffedState)
    {
        case STUFFED_STATE::STUFFED_SPIT:
        {
            CKirby_Body* pBody = pKirby->Get_Body();
            CAnimator* pAnimator = pBody->Get_Animator();
            _float fRatio = pAnimator->Get_Progress();

            if(fRatio >= 0.4f)
                pKirby->Get_Body()->Set_Body(KIRBY_BODY_STATE::NORMAL);
            else if(fRatio >= 0.1f)
                pKirby->Get_Body()->Set_Body(KIRBY_BODY_STATE::INHALE);
            else
                pKirby->Get_Body()->Set_Body(KIRBY_BODY_STATE::STUFFED);

            break;
        }
    }
}

CKirby_Stuffed* CKirby_Stuffed::Create()
{
    CKirby_Stuffed* pInstance = new CKirby_Stuffed();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Stuffed");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Stuffed::Free()
{
    __super::Free();
}