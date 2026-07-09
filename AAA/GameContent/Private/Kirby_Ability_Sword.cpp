#include "Kirby_Ability_Sword.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Kirby_Sword.h"

#include "Effect_Loader.h"

CKirby_Ability_Sword::CKirby_Ability_Sword()
{
}

HRESULT CKirby_Ability_Sword::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"스워드";

    Set_OverlayAni(ABILITY_ANI::WAIT, "Wait", "Sword_HaveSwordWait", "R_ShoulderJ",
        true, false, 1.8f, 0.1f,
        true, false, 1.8f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::RUN, "Run", "HaveSwordMove", "R_ShoulderJ",
        true, false, 3.5f, 0.1f,
        true, false, 3.5f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::FALL, "Fall", "HaveSwordMove", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::LANDING, "Landing", "HaveSwordMove", "R_ShoulderJ",
        false, false, 1.f, 0.05f,
        false, false, 1.f, 1.f, 0.05f, 0.1f);

    // Jump
    Set_OverlayAni(ABILITY_ANI::JUMP_L, "JumpL", "HaveSwordMove", "R_ShoulderJ",
        false, false, 5.f, 0.1f,
        false, false, 5.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_R, "JumpR", "HaveSwordMove", "R_ShoulderJ",
        false, false, 5.f, 0.1f,
        false, false, 5.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_END_L, "JumpEndL", "HaveSwordMove", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_END_R, "JumpEndR", "HaveSwordMove", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 1.f, 0.1f, 0.1f);

    // Hovering
    Set_OverlayAni(ABILITY_ANI::FLIGHT_START, "FlightStart", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, false, 2.25f, 0.1f,
        false, false, 2.25f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT, "Flight", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, true, 2.f, 0.1f,
        false, true, 2.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT_FALL, "FlightFall", "HaveSwordWaitFlight", "R_ShoulderJ",
        true, false, 2.f, 0.1f,
        true, false, 2.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT_LANDING, "FlightLanding", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, false, 2.5f, 0.1f,
        false, false, 2.5f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::AIR_BALL, "AirBall", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, false, 5.f, 0.05f,
        false, false, 5.f, 1.f, 0.05f, 0.1f);

    Set_FullBodyAni(ABILITY_ANI::GUARD, "Sword_Guard", true, true, 0.1f, 1.8f);

    Set_FullBodyAni(ABILITY_ANI::SLIDE_START, "SwordSlideStart", false, false, 0.1f, 1.5f);
    Set_FullBodyAni(ABILITY_ANI::SLIDE, "SwordSlide", true, false, 0.1f, 1.5f);
    Set_FullBodyAni(ABILITY_ANI::SLIDE_END, "SwordSlideEnd", false, false, 0.1f, 1.5f);

    m_fSuperSpinSlashChargeTime = 0.8f;

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Sword::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::SWORD;
}

void CKirby_Ability_Sword::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    SWORD_STATE eStartState = m_eSwordState;

    if (eStartState == SWORD_STATE::END)
    {
        if (pKirby->Get_Movement()->Is_Grounded())
            eStartState = SWORD_STATE::SLASH_1;
        else
            eStartState = SWORD_STATE::JUMP_SLASH_START;
    }

    m_eSwordState = SWORD_STATE::END;

    m_eCurSwordMoveState = SWORD_MOVE_STATE::NONE_MOVE;
    m_ePreSwordMoveState = SWORD_MOVE_STATE::NONE_MOVE;

    m_bReqEndAttackState = false;
    m_bReserveNextAttack = false;
    m_bSpinSlashCharge = false;
    m_bMoveLock = false;

    m_fAccSuperSpinSlashChargeTime = 0.f;
    m_iSuperSpinSlashCount = 7;

    ZeroMemory(&m_vSwordWishDir, sizeof(m_vSwordWishDir));

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_KirbyEye(KIRBY_EYE_STATE::ANGRY);

    Change_SwordState(pKirby, eStartState);
}

void CKirby_Ability_Sword::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_ChargeTime(fTimeDelta);
    Update_SwordState(pKirby, fTimeDelta);

    ChargeAnimationOverlay(pKirby);

    if (m_bMoveLock == false)
        pKirby->Add_MoveDir(m_vSwordWishDir);

    ZeroMemory(&m_vSwordWishDir, sizeof(m_vSwordWishDir));

    m_bSpinSlashCharge = false;

    m_eCurSwordMoveState = SWORD_MOVE_STATE::NONE_MOVE;
}

void CKirby_Ability_Sword::Exit_AttackState(CKirby* pKirby)
{
    Change_SwordState(pKirby, SWORD_STATE::END);

    m_eSwordState = SWORD_STATE::END;

    m_eCurSwordMoveState = SWORD_MOVE_STATE::NONE_MOVE;
    m_ePreSwordMoveState = SWORD_MOVE_STATE::NONE_MOVE;

    m_bReqEndAttackState = true;
    m_bReserveNextAttack = false;
    m_bSpinSlashCharge = false;
    m_bMoveLock = false;

    m_fAccSuperSpinSlashChargeTime = 0.f;

    ZeroMemory(&m_vSwordWishDir, sizeof(m_vSwordWishDir));

    pKirby->Set_RotationLock(false);
    pKirby->Get_Movement()->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);
}

_bool CKirby_Ability_Sword::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        {
            if (!pCommand->IsPress())
                return false;

            m_eCurSwordMoveState = SWORD_MOVE_STATE::MOVE_FRONT;

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            XMStoreFloat3(&m_vSwordWishDir,
                XMVectorAdd(XMLoadFloat3(&pMoveCommand->Get_Dir()), XMLoadFloat3(&m_vSwordWishDir)));

            return true;
        }
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            m_eCurSwordMoveState = SWORD_MOVE_STATE::MOVE_RIGHT;

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            XMStoreFloat3(&m_vSwordWishDir,
                XMVectorAdd(XMLoadFloat3(&pMoveCommand->Get_Dir()), XMLoadFloat3(&m_vSwordWishDir)));

            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (pCommand->IsDown())
            {
                if (m_eSwordState == SWORD_STATE::SLASH_3)
                    return true;

                if (m_eSwordState == SWORD_STATE::JUMP_SLASH)
                {
                    if (pMovement->Is_Grounded() == false)
                        m_bReserveNextAttack = true;
                }
                else
                {
                    m_bReserveNextAttack = true;
                }
            }
            else if (pCommand->IsPress())
            {
                m_bSpinSlashCharge = true;
            }
            else if (pCommand->IsUp())
            {
            }

            return true;
        }
        // Jump Down
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eSwordState == SWORD_STATE::SUPER_SPIN_SLASH_LOOP && pMovement->Is_Grounded())
            {
                pMovement->Try_Jump();

                if (m_iSuperSpinSlashCount > 1)
                    m_iSuperSpinSlashCount = 1;
            }

            return true;
        }
    }

    return false;
}

_bool CKirby_Ability_Sword::Enter_Attack_KeyDown(CKirby* pKirby)
{
    if (pKirby->Get_Movement()->Is_Grounded())
        m_eSwordState = SWORD_STATE::SLASH_1;
    else
        m_eSwordState = SWORD_STATE::JUMP_SLASH_START;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    return true;
}

_bool CKirby_Ability_Sword::Enter_Attack_KeyPress(CKirby* pKirby)
{
    m_eSwordState = SWORD_STATE::SPIN_SLASH_CHARGE;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    m_bSpinSlashCharge = true;

    return true;
}

_bool CKirby_Ability_Sword::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // 무시
    return true;
}

void CKirby_Ability_Sword::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    m_bSpinSlashCharge = false;
    m_fAccSuperSpinSlashChargeTime = 0.f;
    m_bReserveNextAttack = false;

    ZeroMemory(&m_vSwordWishDir, sizeof(m_vSwordWishDir));

    End_SpinSlashEffect(m_pSpinSlash, 0.2f);
    End_SpinSlashEffect(m_pSpinSlashTrail, 0.15f);

    Effect_Stop(m_pSwordChargeEffect);
    Effect_Stop(m_pSwordSuperChargeEffect);

    CKirby_Sword* pSword = static_cast<CKirby_Sword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::SWORD));
    pSword->End_Hit();

    __super::On_Damaged_KirbyState(pKirby, tInfo);
}

_bool CKirby_Ability_Sword::Handle_BodyAnimEvent(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    if (static_cast<EANIM_EVENT>(e.iEventType) != EANIM_EVENT::Hitbox)
        return false;

    enum SWORD_HIT_PARAM
    {
        SLASH1_H, SLASH_2_1_H, SLASH_2_2_H, SLASH_2_3_H, SLASH_2_4_H, SLASH_3_H,
        JUMP_SLASH_H,
        SPIN_SLASH, SUPER_SPIN_SLASH
    };

    if (ePhase == ANIM_EVENT_PHASE::BEGIN)
    {
        ATTACK_INFO tAttackInfo{};
        CKirby_Sword* pSword = static_cast<CKirby_Sword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::SWORD));

        switch (static_cast<SWORD_HIT_PARAM>(e.iIntParam))
        {
            case SWORD_HIT_PARAM::SLASH1_H:
            {
                tAttackInfo.fDamage = 10.f;
                tAttackInfo.fKnockback = 4.5f;
                tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                pSword->Begin_Hit(tAttackInfo);
                break;
            }
            case SWORD_HIT_PARAM::SLASH_2_1_H: 
            case SWORD_HIT_PARAM::SLASH_2_2_H:
            case SWORD_HIT_PARAM::SLASH_2_3_H:
            case SWORD_HIT_PARAM::SLASH_2_4_H:
            {
                tAttackInfo.fDamage = 10.f;
                tAttackInfo.fKnockback = 4.5f;
                tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                pSword->Begin_Hit(tAttackInfo);
                break;
            }
            case SWORD_HIT_PARAM::SLASH_3_H:
            {
                tAttackInfo.fDamage = 200.f;
                tAttackInfo.fKnockback = 10.f;
                tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                pSword->Begin_Hit(tAttackInfo);
                break;
            }
            case SWORD_HIT_PARAM::JUMP_SLASH_H:
            {
                tAttackInfo.fDamage = 10.f;
                tAttackInfo.fKnockback = 4.5f;
                tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                pSword->Begin_Hit(tAttackInfo);
                break;
            }
            case SWORD_HIT_PARAM::SPIN_SLASH:
            {
                tAttackInfo.fDamage = 10.f;
                tAttackInfo.fKnockback = 4.5f;
                tAttackInfo.eHitType = HIT_TYPE::SWORD_SPIN;
                pSword->Begin_Hit(tAttackInfo);
                break;
            }
            case SWORD_HIT_PARAM::SUPER_SPIN_SLASH:
            {
                tAttackInfo.fDamage = 10.f;
                tAttackInfo.fKnockback = 20.f;
                tAttackInfo.eHitType = HIT_TYPE::SWORD_SPIN;
                pSword->Begin_Hit(tAttackInfo);
                break;
            }
        }

        return true;
    }
    
    if (ePhase == ANIM_EVENT_PHASE::END)
    {
        switch (static_cast<SWORD_HIT_PARAM>(e.iIntParam))
        {
            case SWORD_HIT_PARAM::SLASH1_H:
            case SWORD_HIT_PARAM::SLASH_2_1_H:
            case SWORD_HIT_PARAM::SLASH_2_2_H:
            case SWORD_HIT_PARAM::SLASH_2_3_H:
            case SWORD_HIT_PARAM::SLASH_2_4_H:
            case SWORD_HIT_PARAM::SLASH_3_H:
            case SWORD_HIT_PARAM::JUMP_SLASH_H:
            case SWORD_HIT_PARAM::SPIN_SLASH:
            case SWORD_HIT_PARAM::SUPER_SPIN_SLASH:
                CKirby_Sword* pSword = static_cast<CKirby_Sword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::SWORD));
                pSword->End_Hit();
                break;
        }
 
        return true;
    }
    
    return false;
}

void CKirby_Ability_Sword::Update_ChargeTime(_float fTimeDelta)
{
    if (m_eSwordState == SWORD_STATE::SPIN_SLASH_CHARGE && m_bSpinSlashCharge == true)
    {
        m_fAccSuperSpinSlashChargeTime += fTimeDelta;
    }
    else
    {
        m_fAccSuperSpinSlashChargeTime = 0.f;
    }
}

void CKirby_Ability_Sword::MoveLock_Ratio(_float fRatio, _float fRatioStart, _float fRatioEnd)
{
    if (fRatio >= fRatioStart && fRatio < fRatioEnd)
        m_bMoveLock = true;
    else 
        m_bMoveLock = false;
}

void CKirby_Ability_Sword::SetSpeed_Ratio(_float fRatio, _float fRatioStart, _float fRatioEnd, CMovement_Child* pMovement, _float fSpeed)
{
    if (fRatio >= fRatioStart && fRatio < fRatioEnd)
        pMovement->Set_MaxHorizontalSpeed(fSpeed);
    else
        pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
}

void CKirby_Ability_Sword::Change_SwordState(CKirby* pKirby, SWORD_STATE eNext)
{
    if (m_eSwordState == eNext)
        return;

    Exit_SwordState(pKirby, m_eSwordState);

    m_eSwordState = eNext;

    Enter_SwordState(pKirby, m_eSwordState);
}

void CKirby_Ability_Sword::Enter_SwordState(CKirby* pKirby, SWORD_STATE eState)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    CMovement_Child* pMovement = pKirby->Get_Movement();
    CKirby_Sword* pSword = static_cast<CKirby_Sword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::SWORD));

    m_bReqEndAttackState = false;

    switch (eState)
    {
        case SWORD_STATE::END:
        {
            // Shuffle Clear
            Clear_Overlay(pKirby, 1, 0.1f);
            m_bReqEndAttackState = true;
            m_bMoveLock = false;
            pKirby->Set_RotationLock(false);
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            break;
        }
        case SWORD_STATE::SLASH_1:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, 1, 0.1f);

            pAnimator->Play("SideSlash", false, false, 0.1f, 1.5f);

            m_bIsStartEffect[SWORD_EFFECT::SLASH1] = false;
            break;
        }
        case SWORD_STATE::SLASH_1_END:
        {
            pAnimator->Play("SideSlashEnd", false, false, 0.1f, 2.f);
            break;
        }
        case SWORD_STATE::SLASH_2:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, 1, 0.1f);
            pAnimator->Play("MultiswordAttack", false, false, 0.1f, 2.f);

            m_bIsStartEffect[SWORD_EFFECT::SLASH2_1] = false;
            m_bIsStartEffect[SWORD_EFFECT::SLASH2_2] = false;
            m_bIsStartEffect[SWORD_EFFECT::SLASH2_3] = false;
            m_bIsStartEffect[SWORD_EFFECT::SLASH2_4] = false;
            break;
        }
        case SWORD_STATE::SLASH_3:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, 1, 0.1f);

            pAnimator->Play("DecisiveSlash", false, false, 0.1f, 2.f);

            m_bIsStartEffect[SWORD_EFFECT::SLASH3] = false;
            break;
        }
        case SWORD_STATE::JUMP_SLASH_START:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, 1, 0.1f);
            pAnimator->Play("SwordSpinStart", false, false, 0.05f, 10.f);
            pKirby->Set_RotationLock(true);
            break;
        }
        case SWORD_STATE::JUMP_SLASH:
        {
            pAnimator->Play("SwordSpin", false, false, 0.05f, 1.5f);
            m_bIsStartEffect[SWORD_EFFECT::JUMPSLASH] = false;
            break;
        }
        case SWORD_STATE::SPIN_SLASH_CHARGE:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, 1, 0.1f);
            pKirby->Set_RotationLock(true);
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed - 6.f);
            pAnimator->Play("SpinSlashCharge", false, false, 0.05f, 2.5f);

            if (m_eCurSwordMoveState == SWORD_MOVE_STATE::MOVE_FRONT)
                pAnimator->Set_Mask("ShuffleFront", OverlayMasks, std::size(OverlayMasks), true, 1.0f, 0.1f, 0.2f);
            else if (m_eCurSwordMoveState == SWORD_MOVE_STATE::MOVE_RIGHT)
                pAnimator->Set_Mask("ShuffleRight", OverlayMasks, std::size(OverlayMasks), true, 1.0f, 0.1f, 0.2f);

            m_bIsStartEffect[SWORD_EFFECT::SPINSLASH] = false;

            CEffect_Loader::GetInstance()->Spawn(L"SwordChargeEffect", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pSwordChargeEffect);

            break;
        }
        case SWORD_STATE::SPIN_SLASH:
        {
            // Shuffle Clear
            Clear_Overlay(pKirby, 1, 0.1f);
            pKirby->Set_RotationLock(true);
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            pAnimator->Play("SpinSlash", false, false, 0.1f, 2.f);
            break;
        }
        case SWORD_STATE::SPIN_SLASH_END:
        {
            pKirby->Set_RotationLock(true);
            pAnimator->Play("SpinSlashEnd", false, false, 0.1f, 3.f);
            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
        {
            pKirby->Set_RotationLock(true);
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed - 6.f);
            pAnimator->Play("SuperSpinSlashChargeStart", false, false, 0.1f, 2.f);

            CEffect_Loader::GetInstance()->Spawn(L"SwordSuperChargeEffect", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pSwordSuperChargeEffect);

            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
        {
            pKirby->Set_RotationLock(true);
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed - 6.f);
            pAnimator->Play("SuperSpinSlashCharge", true, false, 0.1f, 2.f);

            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_START:
        {
            // Shuffle Clear
            Clear_Overlay(pKirby, 1, 0.1f);
            pKirby->Set_RotationLock(true);
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            pAnimator->Play("SuperSpinSlashStart", false, false, 0.1f, 2.f);
            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
        {
            pKirby->Set_RotationLock(true);
            pAnimator->Play("SuperSpinSlashLoop", false, true, 0.f, 2.f);
            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_END:
        {
            pKirby->Set_RotationLock(true);
            pAnimator->Play("SuperSpinSlashEnd", false, false, 0.f, 3.f);
            break;
        }
    }

    m_bReserveNextAttack = false;
}

void CKirby_Ability_Sword::Update_SwordState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    CMovement_Child* pMovement = pKirby->Get_Movement();

    _float fRatio = pAnimator->Get_Progress();
    _bool bIsAniFinish = pAnimator->Is_Finished();

    switch (m_eSwordState)
    {
        case SWORD_STATE::END:
            m_bReqEndAttackState = true;
            break;

        case SWORD_STATE::SLASH_1:
            MoveLock_Ratio(fRatio, 0.45f, 1.f);

            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    Change_SwordState(pKirby, SWORD_STATE::SLASH_2);
                else if (m_bSpinSlashCharge)
                    Change_SwordState(pKirby, SWORD_STATE::SPIN_SLASH_CHARGE);
                else
                    Change_SwordState(pKirby, SWORD_STATE::SLASH_1_END);
            }

            if (CanPlayEffect(SWORD_EFFECT::SLASH1, pAnimator, 0.45f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SwordSlash1", pKirby->Get_LevelIndex(),
                    _float3(-0.2f, 0.75f, 1.1f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 12.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr());
            }

            break;

        case SWORD_STATE::SLASH_1_END:
            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    Change_SwordState(pKirby, SWORD_STATE::SLASH_2);
                else
                    Change_SwordState(pKirby, SWORD_STATE::END);
            }
            break;

        case SWORD_STATE::SLASH_2:
            MoveLock_Ratio(fRatio, 0.8f, 1.f);
            SetSpeed_Ratio(fRatio, 0.f, 0.8f, pMovement, CKirby::s_fMaxHorizontalSpeed - 2.f);

            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    Change_SwordState(pKirby, SWORD_STATE::SLASH_3);
                else if (m_bSpinSlashCharge)
                    Change_SwordState(pKirby, SWORD_STATE::SPIN_SLASH_CHARGE);
                else
                    Change_SwordState(pKirby, SWORD_STATE::END);
            }

            if (CanPlayEffect(SWORD_EFFECT::SLASH2_1, pAnimator, 0.1f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SwordSlash1", pKirby->Get_LevelIndex(),
                    _float3(0.1f, 0.55f, 1.3f), _float3(0.f, 0.f, 1.f), _float3(0.f, -10.f, 170.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr());
            }
            if (CanPlayEffect(SWORD_EFFECT::SLASH2_2, pAnimator, 0.32f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SwordSlash1", pKirby->Get_LevelIndex(),
                    _float3(-0.2f, 0.9f, 1.1f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 15.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr());
            }
            if (CanPlayEffect(SWORD_EFFECT::SLASH2_3, pAnimator, 0.5f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SwordSlash1", pKirby->Get_LevelIndex(),
                    _float3(0.1f, 0.55f, 1.3f), _float3(0.f, 0.f, 1.f), _float3(0.f, -15.f, 170.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr());
            }
            if (CanPlayEffect(SWORD_EFFECT::SLASH2_4, pAnimator, 0.75f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SwordSlash1", pKirby->Get_LevelIndex(),
                    _float3(-0.2f, 0.9f, 1.1f), _float3(0.f, 0.f, 1.f), _float3(0.f, 20.f, 2.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr());
            }

            break;

        case SWORD_STATE::SLASH_3:
            MoveLock_Ratio(fRatio, 0.6f, 1.f);
            SetSpeed_Ratio(fRatio, 0.f, 0.6f, pMovement, CKirby::s_fMaxHorizontalSpeed + 5.f);

            if (bIsAniFinish)
            {
                if (m_bSpinSlashCharge)
                    Change_SwordState(pKirby, SWORD_STATE::SPIN_SLASH_CHARGE);
                else
                    Change_SwordState(pKirby, SWORD_STATE::END);
            }

            if (CanPlayEffect(SWORD_EFFECT::SLASH3, pAnimator, 0.38f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SwordSlash3", pKirby->Get_LevelIndex(),
                    _float3(0.5f, 0.8f, 0.f), _float3(0.f, 0.f, 1.f), _float3(0.f, 240.f, 0.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr());
            }

            break;

        // Jump
        case SWORD_STATE::JUMP_SLASH_START:
            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::JUMP_SLASH);
            break;

        case SWORD_STATE::JUMP_SLASH:
            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                {
                    if (pMovement->Is_Grounded())
                        Change_SwordState(pKirby, SWORD_STATE::SLASH_1);
                    else
                        Change_SwordState(pKirby, SWORD_STATE::JUMP_SLASH_START);
                }
                else
                {
                    Change_SwordState(pKirby, SWORD_STATE::END);
                }
            }

            if (CanPlayEffect(SWORD_EFFECT::JUMPSLASH, pAnimator, 0.01f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"JumpSlash_1", pKirby->Get_LevelIndex(),
                    _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 90.f, 0.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr());
            }

            break;

        // Charge
        case SWORD_STATE::SPIN_SLASH_CHARGE:
            if (m_fAccSuperSpinSlashChargeTime >= m_fSuperSpinSlashChargeTime)
            {
                Change_SwordState(pKirby, SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START);
            }
            else if (m_bSpinSlashCharge == false)
            {
                if (bIsAniFinish)
                    Change_SwordState(pKirby, SWORD_STATE::SPIN_SLASH);
                else
                    Change_SwordState(pKirby, SWORD_STATE::END);
            }
            break;

        // Spin
        case SWORD_STATE::SPIN_SLASH:
        {
            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::SPIN_SLASH_END);

            if (CanPlayEffect(SWORD_EFFECT::SPINSLASH, pAnimator, 0.01f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SpinSlash", pKirby->Get_LevelIndex(),
                    _float3(0.f, 1.f, 0.f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 0.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pSpinSlash);
                m_pSpinSlash->Set_EffectPartPlay(L"Proto_Common_SpinSlash_1", false);

                CEffect_Loader::GetInstance()->Spawn(L"SpinSlashTrail", pKirby->Get_LevelIndex(),
                    _float3(0.f, 1.05f, 0.f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 0.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pSpinSlashTrail);
            }

            if (pAnimator->Get_Progress() >= 0.78f)
            {
                End_SpinSlashEffect(m_pSpinSlash, 0.2f);
                End_SpinSlashEffect(m_pSpinSlashTrail, 0.15f);
            }

            break;
        }

        case SWORD_STATE::SPIN_SLASH_END:
            MoveLock_Ratio(fRatio, 0.f, 0.75f);

            if (fRatio >= 0.75f)
                pKirby->Set_RotationLock(false);

            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::END);
            break;

        //  Charge Super
        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::SUPER_SPIN_SLASH_CHARGE);
            break;

        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
            if (m_bSpinSlashCharge == false)
                Change_SwordState(pKirby, SWORD_STATE::SUPER_SPIN_SLASH_START);
            break;

        // Spin Super
        case SWORD_STATE::SUPER_SPIN_SLASH_START:
        {
            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::SUPER_SPIN_SLASH_LOOP);

            if (CanPlayEffect(SWORD_EFFECT::SPINSLASH, pAnimator, 0.15f))
            {
                CEffect_Loader::GetInstance()->Spawn(L"SpinSlash", pKirby->Get_LevelIndex(),
                    _float3(0.f, 1.05f, 0.f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 0.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pSpinSlash);

                CEffect_Loader::GetInstance()->Spawn(L"SpinSlashTrail_Super", pKirby->Get_LevelIndex(),
                    _float3(0.f, 1.05f, 0.f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 0.f),
                    pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pSpinSlashTrail);
            }
            break;
        }

        case SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
            if (bIsAniFinish)
            {
                if (m_iSuperSpinSlashCount > 0)
                {
                    --m_iSuperSpinSlashCount;
                    pAnimator->Play("SuperSpinSlashLoop", false, true, 0.1f, 2.f);
                }
                else
                {
                    Change_SwordState(pKirby, SWORD_STATE::SUPER_SPIN_SLASH_END);
                }
            }

            break;

        case SWORD_STATE::SUPER_SPIN_SLASH_END:
            MoveLock_Ratio(fRatio, 0.f, 0.75f);

            if (fRatio >= 0.75f)
                pKirby->Set_RotationLock(false);

            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::END);

            break;
        }
}

void CKirby_Ability_Sword::Exit_SwordState(CKirby* pKirby, SWORD_STATE eState)
{
    m_bMoveLock = false;

    pKirby->Get_Movement()->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eSwordState)
    {
        case END:
        case SLASH_1:
        case SLASH_1_END:
        case SLASH_2:
        case SLASH_3:
            break;

        case JUMP_SLASH_START:
            break;
        case JUMP_SLASH:
            pKirby->Set_RotationLock(false);
            break;

        case SPIN_SLASH_CHARGE:
            Effect_Stop(m_pSwordChargeEffect);
            break;
        case SPIN_SLASH:
            break;
        case SPIN_SLASH_END:
            break;
        case SUPER_SPIN_SLASH_CHARGE_START:
            break;
        case SUPER_SPIN_SLASH_CHARGE:
            Effect_Stop(m_pSwordSuperChargeEffect);
            break;
        case SUPER_SPIN_SLASH_START:
            break;
        case SUPER_SPIN_SLASH_LOOP:
            End_SpinSlashEffect(m_pSpinSlash, 0.2f);
            End_SpinSlashEffect(m_pSpinSlashTrail, 0.15f);
            break;
        case SUPER_SPIN_SLASH_END:
            break;
    }
}

_bool CKirby_Ability_Sword::Has_SwordMoveDir()
{
    _vector vSwordWishDir = XMLoadFloat3(&m_vSwordWishDir);

    if (XMVector3Equal(vSwordWishDir, XMVectorZero()))
        return false;

    return true;
}

void CKirby_Ability_Sword::ChargeAnimationOverlay(CKirby* pKirby)
{
    if (m_eCurSwordMoveState != m_ePreSwordMoveState)
    {
        CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

        switch (m_eSwordState)
        {
            case SWORD_STATE::SPIN_SLASH_CHARGE:
            case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
            case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
                if (m_eCurSwordMoveState == SWORD_MOVE_STATE::NONE_MOVE)
                {
                    pAnimator->Pause_Mask();
                }
                else if (m_eCurSwordMoveState == SWORD_MOVE_STATE::MOVE_FRONT)
                {
                    pAnimator->Set_Mask("ShuffleFront", OverlayMasks, std::size(OverlayMasks), true, 1.0f, 0.1f, 0.2f);
                    pAnimator->Resume_Mask();
                }
                else if (m_eCurSwordMoveState == SWORD_MOVE_STATE::MOVE_RIGHT)
                {
                    pAnimator->Set_Mask("ShuffleRight", OverlayMasks, std::size(OverlayMasks), true, 1.0f, 0.1f, 0.2f);
                    pAnimator->Resume_Mask();
                }
                break;
        }

        m_ePreSwordMoveState = m_eCurSwordMoveState;
    }
}

_bool CKirby_Ability_Sword::CanPlayEffect(SWORD_EFFECT eSwordEffect, CAnimator* pAnimator, _float fRatio)
{
    if (m_bIsStartEffect[eSwordEffect] == true)
        return false;

    _float fCurAniRatio = pAnimator->Get_Progress();

    if (fCurAniRatio < fRatio)
        return false;

    m_bIsStartEffect[eSwordEffect] = true;

    return true;
}

void CKirby_Ability_Sword::End_SpinSlashEffect(CEffect_Container*& pEffectContainer, _float fFadeOutDuration)
{
    if (pEffectContainer != nullptr)
    {
        pEffectContainer->Start_FadeOut(fFadeOutDuration);
        pEffectContainer = nullptr;
    }
}

CKirby_Ability_Sword* CKirby_Ability_Sword::Create()
{
    CKirby_Ability_Sword* pInstance = new CKirby_Ability_Sword();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Sword::Free()
{
    __super::Free();
}