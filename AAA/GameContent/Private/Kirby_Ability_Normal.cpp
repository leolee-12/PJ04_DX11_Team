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

    m_bReqEndAttackState = false;

    // Speed
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    pMovementCom->Set_MaxHorizontalSpeed(2.f);
}

ABILITY_UPDATE_RESULT CKirby_Ability_Normal::Update_Ability(CKirby* pKirby, _float fTimeDelta)
{
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    _float fYVelocity = pMovementCom->Get_VerticalVelocity();

    _bool bIsGround = pMovementCom->Is_Grounded();

    //if (bIsGround == false && fYVelocity <= CKirby::s_fFallVelocityY)
    if (bIsGround == false)
        m_eCurMoveState = INHALE_MOVE_STATE::FALL;
    else if (pKirby->Has_MoveDir() == true)
        m_eCurMoveState = INHALE_MOVE_STATE::WALK;
    else
        m_eCurMoveState = INHALE_MOVE_STATE::WAIT;    

    // Test Code
    if (Change_Ability(pKirby) == true)
        return ABILITY_UPDATE_RESULT::ABILITY_CHANGED;

    // Super Inhale Timer
    if (m_AccSuperInHaleTime < m_MaxSuperInHaleTime)
        m_AccSuperInHaleTime += fTimeDelta;

    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();

    // Inhale 종료
    if (m_eInhaleState != INHALE_STATE::INHALE_END && m_bReqInhale == true)
    {
        m_eInhaleState = INHALE_STATE::INHALE_END;
        pAnimator->Play("InhaleEnd", false, false, 0.1f, 1.5f);
        pMovementCom->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
    }
    if (m_eInhaleState == INHALE_STATE::INHALE_END)
    {
        if (pAnimator->Get_Progress() >= 0.5f)
            pKirby_Body->Set_Body(KIRBY_BODY_STATE::NORMAL);

        if (pAnimator->Is_Finished() == true)
        {
            pKirby_Body->Set_Eye(KIRBY_EYE_STATE::IDLE);
            m_bReqEndAttackState = true;
        }

        return ABILITY_UPDATE_RESULT::NONE;
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

    return ABILITY_UPDATE_RESULT::NONE;
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

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            pKirby->Add_MoveDir(pMoveCommand->Get_Dir());
            return true;
        }

        // Attack Up
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsUp())
                return false;

            m_bReqInhale = true;
            return true;
        }
    }

    return false;
}

_bool CKirby_Ability_Normal::Enter_Attack_KeyDown(CKirby* pKirby)
{
    m_bReqInhale = false;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    return true;
}

_bool CKirby_Ability_Normal::Enter_Attack_KeyPress(CKirby* pKirby)
{
    // 무시
    return true;
}

_bool CKirby_Ability_Normal::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // 무시
    return true;
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

_bool CKirby_Ability_Normal::Change_Ability(CKirby* pKirby)
{
    //Test Code
    if (GetAsyncKeyState('T') & 0x8000)
    {
        Reset_Default(pKirby);

        // 먹은 오브젝트에서 가져온다.
        KIRBY_ABILITY_TYPE eAbilityType = KIRBY_ABILITY_TYPE::SWORD;
        pKirby->Set_KirbyAbility(eAbilityType);

        pKirby->Change_State(KIRBY_STATE_TYPE::GET_ABILITY);

        return true;
    }

    return false;
}

void CKirby_Ability_Normal::Reset_Default(CKirby* pKirby)
{
    // Test Code
    CMovement_Child* pMovement = pKirby->Get_Movement();

    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    pKirby_Body->Set_Eye(KIRBY_EYE_STATE::IDLE);
    pKirby_Body->Set_Body(KIRBY_BODY_STATE::NORMAL);
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