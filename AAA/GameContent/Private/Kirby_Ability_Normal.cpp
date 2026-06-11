#include "Kirby_Ability_Normal.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Ability_Normal::CKirby_Ability_Normal()
{
}

HRESULT CKirby_Ability_Normal::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_MaxSuperInHaleTime = 1.f;

    return S_OK;
}

KIRBY_ABILITY_TYPE CKirby_Ability_Normal::Get_AbilityType()
{
    return KIRBY_ABILITY_TYPE::NORMAL;
}

void CKirby_Ability_Normal::Enter_Ability(CKirby* pKirby)
{
    // Inhale State
    m_eInhaleState = INHALE_STATE::INHALE_LOOP;

    // Super Inhale Timer
    m_AccSuperInHaleTime = 0.f;

    // Inhale Animation
    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();
    
    _string strAniName;
    Choose_InhaleAniName(strAniName);

    pAnimator->Play(strAniName, true, false, 0.1f, 1.5f);

    // Inhale Body
    pKirby_Body->Set_Body(KIRBY_BODY_STATE::INHALE);

    m_bEndAttack = false;

    // Speed
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    pMovementCom->Set_MaxHorizontalSpeed(2.f);
}

void CKirby_Ability_Normal::Update_Ability(CKirby* pKirby, _float fTimeDelta)
{
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    _float fYVelocity = pMovementCom->Get_VerticalVelocity();

    _bool bIsGround = pMovementCom->Is_Grounded();
    if (bIsGround == false && fYVelocity <= CKirby::s_fFallVelocityY)
    {
        m_eCurMoveState = INHALE_MOVE_STATE::FALL;
    }
    else if (pKirby->Has_MoveDir() == false)
    {
        m_eCurMoveState = INHALE_MOVE_STATE::WAIT;
    }

    // Super Inhale Timer
    if (m_AccSuperInHaleTime < m_MaxSuperInHaleTime)
        m_AccSuperInHaleTime += fTimeDelta;

    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();

    // Inhale 종료
    if (m_eInhaleState != INHALE_STATE::INHALE_END && pKirby->Get_KirbyAbility()->IsFinished() == true)
    {
        m_eInhaleState = INHALE_STATE::INHALE_END;
        pAnimator->Play("InhaleEnd", false, false, 0.1f, 1.5f);
        pMovementCom->Set_MaxHorizontalSpeed(8.f);
    }
    if (m_eInhaleState == INHALE_STATE::INHALE_END)
    {
        if (pAnimator->Get_Progress() >= 0.5f)
            pKirby_Body->Set_Body(KIRBY_BODY_STATE::NORMAL);

        if (pAnimator->Is_Finished() == true)
        {
            pKirby_Body->Set_Eye(KIRBY_EYE_STATE::IDLE);
            m_bEndAttack = true;
        }
        return;
    }

    // Inhale 강화
    if ((m_eInhaleState == INHALE_STATE::INHALE_LOOP && m_AccSuperInHaleTime >= m_MaxSuperInHaleTime)||
        m_bForceEnterSuperInhaleStart == true)
    {
        m_eInhaleState = INHALE_STATE::SUPER_INHALE_START;
        pAnimator->Play("SuperInhaleStart", false, false, 0.1f, 2.5f);
        pKirby_Body->Set_Eye(KIRBY_EYE_STATE::ANGRY);
        m_bForceEnterSuperInhaleStart = false;
    }
    else if (m_eInhaleState == INHALE_STATE::SUPER_INHALE_START &&
        pAnimator->Is_Finished() == true)
    {
        m_eInhaleState = INHALE_STATE::SUPER_INHALE_LOOP;

        _string strAniName;
        Choose_InhaleAniName(strAniName);
        pAnimator->Play(strAniName, true, false, 0.1f, 1.5f);
        
        pKirby_Body->Set_Eye(KIRBY_EYE_STATE::CLOSE);
    }

    Interpolation_Inhale(pAnimator);
}

void CKirby_Ability_Normal::Exit_Ability(CKirby* pKirby)
{
}

_bool CKirby_Ability_Normal::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
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

            m_eCurMoveState = INHALE_MOVE_STATE::WALK;
            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            pKirby->Add_MoveDir(pMoveCommand->Get_Dir());
            return true;
        }
    }

    return false;
}

void CKirby_Ability_Normal::Down_Attack(CKirby* pKirby)
{
    m_bIsFinished = false;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
}

void CKirby_Ability_Normal::Up_Attack(CKirby* pKirby)
{
    m_bIsFinished = true;
}

_bool CKirby_Ability_Normal::Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation)
{
    switch (eAttackLocation)
    {
        case KIRBY_ATTACK_LOCATION::GROUND:     return true;
        case KIRBY_ATTACK_LOCATION::AIR:        return false;
    }

    return false;
}

void CKirby_Ability_Normal::Interpolation_Inhale(CAnimator* pAnimator)
{
    if (m_eCurMoveState != m_ePreMoveState)
    {
        _string strAniName;

        switch (m_eInhaleState)
        {
            case INHALE_STATE::INHALE_LOOP:
            {
                if (m_eCurMoveState == INHALE_MOVE_STATE::WAIT)         strAniName = "Inhale";
                else if (m_eCurMoveState == INHALE_MOVE_STATE::WALK)    strAniName = "InhaleWalk";
                else if (m_eCurMoveState == INHALE_MOVE_STATE::FALL)    strAniName = "InhaleFall";
                break;
            }

            case INHALE_STATE::SUPER_INHALE_LOOP:
            {
                if (m_eCurMoveState == INHALE_MOVE_STATE::WAIT)         m_bForceEnterSuperInhaleStart = true;
                else if (m_eCurMoveState == INHALE_MOVE_STATE::WALK)    strAniName = "SuperInhaleWalk";            
                else if (m_eCurMoveState == INHALE_MOVE_STATE::FALL)    strAniName = "SuperInhaleFall";
                break;
            }
        }

        pAnimator->Play(strAniName, true, false, 0.05f, 1.5f);

        m_ePreMoveState = m_eCurMoveState;
    }
}

void CKirby_Ability_Normal::Choose_InhaleAniName(_string& strAniName)
{
    switch (m_eInhaleState)
    {
        case INHALE_STATE::INHALE_LOOP:
        {
            if (m_eCurMoveState == INHALE_MOVE_STATE::WAIT)         strAniName = "Inhale";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::WALK)    strAniName = "InhaleWalk";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::FALL)    strAniName = "InhaleFall";
            break;
        }

        case INHALE_STATE::SUPER_INHALE_LOOP:
        {
            if (m_eCurMoveState == INHALE_MOVE_STATE::WAIT)         strAniName = "SuperInhale";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::WALK)    strAniName = "SuperInhaleWalk";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::FALL)    strAniName = "SuperInhaleFall";
            break;
        }
    }
}

CKirby_Ability_Normal* CKirby_Ability_Normal::Create()
{
    CKirby_Ability_Normal* pInstance = new CKirby_Ability_Normal();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Normal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Normal::Free()
{
    __super::Free();
}
