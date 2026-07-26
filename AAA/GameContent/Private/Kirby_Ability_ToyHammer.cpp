#include "Kirby_Ability_ToyHammer.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Movement_Child.h"

#include "Kirby_ToyHammer.h"

#include "Effect_Loader.h"

namespace
{
     constexpr _float fHammerMaxHorizontalSpeed = 2.f;
     constexpr _float fAttackFinalMaxHorizontalSpeed = 12.f;
     constexpr _float fChargeAttack3MaxHorizontalSpeed = 12.f;

     constexpr _float fChargeLevel2Time = 0.58f;
     constexpr _float fChargeLevel3Time = 1.4933333f;
     constexpr _float fChargeLevel4Time = 8.8f;

     constexpr _uint iChargeOverlaySlot = 2;
}

CKirby_Ability_ToyHammer::CKirby_Ability_ToyHammer()
{
}

HRESULT CKirby_Ability_ToyHammer::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"»Ð¸ÁÄ¡";

    Set_OverlayAni(ABILITY_ANI::WAIT, "Wait", "HaveHammerWait", "R_ShoulderJ",
        true, false, 1.8f, 0.1f,
        true, false, 1.8f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::RUN, "Run", "HaveHammerWait", "R_ShoulderJ",
        true, false, 3.5f, 0.1f,
        true, false, 3.5f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::FALL, "Fall", "HaveHammerWait", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::LANDING, "Landing", "HaveHammerWait", "R_ShoulderJ",
        false, false, 1.f, 0.05f,
        false, false, 1.f, 1.f, 0.05f, 0.1f);

    // Jump
    Set_OverlayAni(ABILITY_ANI::JUMP_L, "JumpL", "HaveHammerWait", "R_ShoulderJ",
        false, false, 5.f, 0.1f,
        false, false, 5.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_R, "JumpR", "HaveHammerWait", "R_ShoulderJ",
        false, false, 5.f, 0.1f,
        false, false, 5.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_END_L, "JumpEndL", "HaveHammerWait", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_END_R, "JumpEndR", "HaveHammerWait", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 1.f, 0.1f, 0.1f);

    // Hovering
    Set_OverlayAni(ABILITY_ANI::FLIGHT_START, "FlightStart", "HaveHammerWait", "R_ShoulderJ",
        false, false, 2.25f, 0.1f,
        false, false, 2.25f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT, "Flight", "HaveHammerWait", "R_ShoulderJ",
        false, true, 2.f, 0.1f,
        false, true, 2.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT_FALL, "FlightFall", "HaveHammerWait", "R_ShoulderJ",
        true, false, 2.f, 0.1f,
        true, false, 2.f, 1.f, 0.1f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT_LANDING, "FlightLanding", "HaveHammerWait", "R_ShoulderJ",
        false, false, 2.5f, 0.1f,
        false, false, 2.5f, 1.f, 0.1f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::AIR_BALL, "AirBall", "HaveHammerWait", "R_ShoulderJ",
        false, false, 5.f, 0.05f,
        false, false, 5.f, 1.f, 0.05f, 0.1f);

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_ToyHammer::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::TOY_HAMMER;
}

void CKirby_Ability_ToyHammer::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = false;
    m_iNormalAttackCount = 0;

    Clear_Overlay(pKirby, 1, 0.f);

    if (m_eToyHammerStartState != TOY_HAMMER_STATE::WHEELHAMMER)
    {
        CMovement_Child* pMovement = pKirby->Get_Movement();
        pMovement->Set_MaxHorizontalSpeed(fHammerMaxHorizontalSpeed);
    }

    m_eToyHammerState = TOY_HAMMER_STATE::TOY_HAMER_STATE_END;
    Change_ToyHammerState(pKirby, m_eToyHammerStartState);
    m_eToyHammerStartState = TOY_HAMMER_STATE::TOY_HAMER_STATE_END;
}

void CKirby_Ability_ToyHammer::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_ToyHammerState(pKirby, fTimeDelta);

    m_bIsCharging = false;
    m_bWheelHammerPressing = false;
}

void CKirby_Ability_ToyHammer::Exit_AttackState(CKirby* pKirby)
{
    Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::NONE);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    m_bIsCharging = false;
    m_fChargeTime = 0.f;
    m_bWheelHammerPressing = false;
    m_bReserveNextAttack = false;
    m_iNormalAttackCount = 0;

    CKirby_ToyHammer* pToyHammer = static_cast<CKirby_ToyHammer*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::TOY_HAMMER));
    pToyHammer->End_Hit();
    pToyHammer->BurnHammer(false);
    CAnimator* pToyHammerAnimator = pToyHammer->Get_Animator();
    pToyHammerAnimator->Play("Reset", true, true, 0.05f, 1.5f);
}

_bool CKirby_Ability_ToyHammer::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

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
        // Jump
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            if ((m_eToyHammerState == TOY_HAMMER_STATE::CHARGE_START ||
                m_eToyHammerState == TOY_HAMMER_STATE::CHARGING) &&
                pMovement->Try_Jump())
            {
                Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::JUMP_START);
            }

            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (pCommand->IsDown())
            {
                CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
                if ((m_eToyHammerState == TOY_HAMMER_STATE::ATTACK_END && pAnimator->Get_Progress() >= 0.8f) ||
                    m_eToyHammerState == TOY_HAMMER_STATE::ATTACK_END)
                    m_bReserveNextAttack = true;
            }
            else if (pCommand->IsPress())
            {
                m_bIsCharging = true;
                m_bWheelHammerPressing = true;
            }
            else if (pCommand->IsUp())
            {
            }

            return true;
        }
    }

    return false;
}

_bool CKirby_Ability_ToyHammer::Enter_Attack_KeyDown(CKirby* pKirby)
{
    if (pKirby->Get_Movement()->Is_Grounded())
        m_eToyHammerStartState = TOY_HAMMER_STATE::CHARGE_START;
    else
        m_eToyHammerStartState = TOY_HAMMER_STATE::WHEELHAMMER;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    return true;
}

_bool CKirby_Ability_ToyHammer::Enter_Attack_KeyPress(CKirby* pKirby)
{
    if (pKirby->Get_Movement()->Is_Grounded())
    {
        m_bIsCharging = true;

        m_eToyHammerStartState = TOY_HAMMER_STATE::CHARGE_START;
        pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
    }

    return true;
}

_bool CKirby_Ability_ToyHammer::Enter_Attack_KeyUp(CKirby* pKirby)
{
    return true;
}

void CKirby_Ability_ToyHammer::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    __super::On_Damaged_KirbyState(pKirby, tInfo);
}

_bool CKirby_Ability_ToyHammer::Handle_BodyAnimEvent(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    if (__super::Handle_BodyAnimEvent(pKirby, e, ePhase))
        return true;

    if (static_cast<EANIM_EVENT>(e.iEventType) != EANIM_EVENT::Hitbox)
        return false;

    CKirby_ToyHammer* pToyHammer = static_cast<CKirby_ToyHammer*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::TOY_HAMMER));

    if (pToyHammer == nullptr)
        return false;

    enum TOY_HAMMER_HIT_PARAM
    {
        HAMMER_ATTACK_H,
        HAMMER_ATTACK_FINAL_H,
        CHARGE_ATTACK_1_H,
        CHARGE_ATTACK_2_H,
        CHARGE_ATTACK_3_H,
        CHARGE_ATTACK_4_H,
        WHEELHAMMER_H,
        WHEELHAMMER_FALL_H
    };

    _int iHitParam = e.iIntParam;

    if (iHitParam == CHARGE_ATTACK_1_H && m_eToyHammerState == TOY_HAMMER_STATE::CHARGE_ATTACK_4)
        iHitParam = CHARGE_ATTACK_4_H;

    if (ePhase == ANIM_EVENT_PHASE::BEGIN)
    {
        ATTACK_INFO tAttackInfo{};

        switch (static_cast<TOY_HAMMER_HIT_PARAM>(iHitParam))
        {
            case HAMMER_ATTACK_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::HAMMER_ATTACK);
                tAttackInfo.fDamage = 100.f;
                tAttackInfo.fKnockback = 5.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_NORMAL;
                break;
            case HAMMER_ATTACK_FINAL_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::HAMMER_ATTACK_FINAL);
                tAttackInfo.fDamage = 100.f;
                tAttackInfo.fKnockback = 10.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_PRESS;
                break;
            case CHARGE_ATTACK_1_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_1);
                tAttackInfo.fDamage = 100.f;
                tAttackInfo.fKnockback = 5.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_NORMAL;
                break;
            case CHARGE_ATTACK_2_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_2);
                tAttackInfo.fDamage = 150.f;
                tAttackInfo.fKnockback = 10.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_NORMAL;
                break;
            case CHARGE_ATTACK_3_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_3);
                tAttackInfo.fDamage = 200.f;
                tAttackInfo.fKnockback = 15.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_NORMAL;
                break;
            case CHARGE_ATTACK_4_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_4);
                tAttackInfo.fDamage = 100.f;
                tAttackInfo.fKnockback = 5.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_NORMAL;
                break;
            case WHEELHAMMER_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::WHEELHAMMER);
                tAttackInfo.fDamage = 100.f;
                tAttackInfo.fKnockback = 5.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_NORMAL;
                break;
            case WHEELHAMMER_FALL_H:
                pToyHammer->Change_HitBox(TOY_HAMMER_HITBOX_TYPE::WHEELHAMMER_FALL);
                tAttackInfo.fDamage = 100.f;
                tAttackInfo.fKnockback = 5.f;
                tAttackInfo.eHitType = HIT_TYPE::HAMMER_PRESS;
                break;
            default:
                return false;
        }

        pToyHammer->Begin_Hit(tAttackInfo);
        return true;
    }

    if (ePhase == ANIM_EVENT_PHASE::END)
    {
        switch (static_cast<TOY_HAMMER_HIT_PARAM>(iHitParam))
        {
            case HAMMER_ATTACK_H:
            case HAMMER_ATTACK_FINAL_H:
            case CHARGE_ATTACK_1_H:
            case CHARGE_ATTACK_2_H:
            case CHARGE_ATTACK_3_H:
            case CHARGE_ATTACK_4_H:
            case WHEELHAMMER_H:
            case WHEELHAMMER_FALL_H:
                pToyHammer->End_Hit();
                return true;
        }
    }

    return false;
}

void CKirby_Ability_ToyHammer::Change_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eNext)
{
    if (m_eToyHammerState == eNext)
        return;

    Exit_ToyHammerState(pKirby, m_eToyHammerState);

    m_eToyHammerState = eNext;

    Enter_ToyHammerState(pKirby, m_eToyHammerState);
}

void CKirby_Ability_ToyHammer::Enter_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eState)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    CKirby_ToyHammer* pToyHammer = static_cast<CKirby_ToyHammer*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::TOY_HAMMER));
    CAnimator* pToyHammerAnimator = pToyHammer->Get_Animator();

    switch (eState)
    {
        case TOY_HAMMER_STATE::ATTACK_START:
        {
            Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::NONE);

            pAnimator->Play("HammerAttackStartToy", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("HammerAttackStart", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK:
        {
            Clear_Overlay(pKirby, 1, 0.f);
            ++m_iNormalAttackCount;
            pAnimator->Play("HammerAttackToy", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("HammerAttack", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK_END:
        {
            pAnimator->Play("HammerAttackHitToy", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("HammerAttackHit", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK_MISS:
        {
            //m_iNormalAttackCount = 0;
            //pAnimator->Play("HammerAttackMiss", false, true, 0.1f, 2.5f);
            //pToyHammerAnimator->Play("HammerAttackMiss", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK_FINAL:
        {
            m_bAttackFinalEndOverlayApplied = false;
            m_bAttackFinalAddVelocity = false;
            pAnimator->Play("HammerAttackFinalToy", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("HammerAttackFinal", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_START:
        {
            m_bAttackEndOverlayApplied = false;
            pAnimator->Play("OnigorosiHammerStart", false, true, 0.f, 3.f);
            pToyHammerAnimator->Play("OnigorosiHammerStart", false, true, 0.f, 3.f);
            Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::NONE);
            break;
        }
        case TOY_HAMMER_STATE::CHARGING:
        {
            m_eChargeLevel = CHARGE_LEVEL::LV1;
            m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_Charge1.wav", 0.25f);
            pAnimator->Play("OnigorosiHammerCharge", true, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("OnigorosiHammerCharge", true, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_1:
        {
            m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_OnigorosiAttack1.wav", 0.25f);
            pAnimator->Play("OnigorosiHammerFirst", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("OnigorosiHammerFirst", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_2:
        {
            m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_OnigorosiAttack2.wav", 0.25f);
            pAnimator->Play("OnigorosiHammerSecond", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("OnigorosiHammerSecond", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_3:
        {
            m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_OnigorosiAttack3.wav", 0.25f);
            pAnimator->Play("OnigorosiHammerEnd", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("OnigorosiHammerEnd", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_4:
        {
            m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_AttackFailure1.wav", 0.25f);
            pAnimator->Play("OnigorosiHammerFirst", false, true, 0.1f, 2.5f);
            pToyHammerAnimator->Play("Burst", false, true, 0.1f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::WHEELHAMMER:
        {
            pAnimator->Play("WheelHammer", false, true, 0.f, 2.5f);
            pToyHammerAnimator->Play("WheelHammer", false, true, 0.f, 2.5f);
            break;
        }
        case TOY_HAMMER_STATE::WHEELHAMMER_END:
        {
            m_bWheelHammerEndOverlayApplied = false;
            pAnimator->Play("WheelHammerEnd", false, true, 0.03f, 1.5f);
            pToyHammerAnimator->Play("WheelHammerEnd", false, true, 0.03f, 1.5f);
            break;
        }
        case TOY_HAMMER_STATE::WHEELHAMMER_FALL:
        {
            pAnimator->Play("WheelHammerFall", false, true, 0.03f, 3.f);
            pToyHammerAnimator->Play("WheelHammerFall", false, true, 0.03f, 3.f);
            break;
        }
        case TOY_HAMMER_STATE::REBOUND:
        {
            //// ÀÓ½Ã
            //CMovement_Child* pMovement = pKirby->Get_Movement();
            //pMovement->Set_VelocityY(20.f);
            break;
        }
        case TOY_HAMMER_STATE::TOY_HAMER_STATE_END:
        {
            m_bReqEndAttackState = true;
            break;
        }
    }
}

void CKirby_Ability_ToyHammer::Update_ToyHammerState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    auto AniEndChangeState = [this, pKirby, pAnimator](TOY_HAMMER_STATE eState)->bool
        {
            if (!pAnimator->Is_Finished())
                return false;

            Change_ToyHammerState(pKirby, eState);
            return true;
        };

    switch (m_eToyHammerState)
    {
        case TOY_HAMMER_STATE::ATTACK_START:
        {
            AniEndChangeState(TOY_HAMMER_STATE::ATTACK);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK:
        {
            AniEndChangeState(TOY_HAMMER_STATE::ATTACK_END);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK_END:
        {
            const _float fRatio = pAnimator->Get_Progress();

            if (fRatio >= 0.5f && m_bReserveNextAttack)
            {
                if (m_iNormalAttackCount >= 4)
                    Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::ATTACK_FINAL);
                else
                    Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::ATTACK);
                m_bReserveNextAttack = false;
                return;
            }

            AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END);

            if (!m_bAttackEndOverlayApplied && fRatio >= 0.8f)
            {
                pAnimator->Set_Mask("HaveHammerWait", "R_ShoulderJ", true, 1.f, 0.1f, 0.0f);
                m_bAttackEndOverlayApplied = true;
            }

            break;
        }
        case TOY_HAMMER_STATE::ATTACK_MISS:
        {
            //if (pAnimator->Is_Finished())
            //{
            //    if (m_bReserveNextAttack)
            //    {
            //        Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::ATTACK_START);
            //        m_bReserveNextAttack = false;
            //    }
            //    else
            //    {
            //        Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::TOY_HAMER_STATE_END);
            //    }
            //}

            break;
        }
        case TOY_HAMMER_STATE::ATTACK_FINAL:
        {
            AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END);

            const _float fRatio = pAnimator->Get_Progress();
            if (!m_bAttackFinalEndOverlayApplied && fRatio >= 0.9f)
            {
                pAnimator->Set_Mask("HaveHammerWait", "R_ShoulderJ", true, 1.f, 0.1f, 0.0f);
                m_bAttackFinalEndOverlayApplied = true;
            }

            if (fRatio >= 0.3f)
            {
                pKirby->Reset_MoveDir();

                if(!m_bAttackFinalAddVelocity)
                {
                    CMovement_Child* pMovement = pKirby->Get_Movement();
                    pMovement->Set_MaxHorizontalSpeed(fAttackFinalMaxHorizontalSpeed);

                    MoveLookDir(pKirby, 30.f);

                    m_bAttackFinalAddVelocity = true;
                }
            }

            break;
        }
        case TOY_HAMMER_STATE::CHARGE_START:
        {
            Update_ChargeOverlayAni(pKirby, false, fTimeDelta);

            if(!m_bIsCharging)
                Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::ATTACK_START);
            else 
                AniEndChangeState(TOY_HAMMER_STATE::CHARGING);

            break;
        }
        case TOY_HAMMER_STATE::CHARGING:
        {
            Update_ChargeOverlayAni(pKirby, true, fTimeDelta);

            if (!m_bIsCharging)
            {
                switch (m_eChargeLevel)
                {
                    case CHARGE_LEVEL::LV1:
                        Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::CHARGE_ATTACK_1);
                        break;

                    case CHARGE_LEVEL::LV2:
                        Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::CHARGE_ATTACK_2);
                        break;

                    case CHARGE_LEVEL::LV3:
                        Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::CHARGE_ATTACK_3);
                        break;

                    case CHARGE_LEVEL::LV4:
                        Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::CHARGE_ATTACK_4);
                        break;
                }

                return;
            }

            m_fChargeTime += fTimeDelta;

            if (m_eChargeLevel == CHARGE_LEVEL::LV1 && m_fChargeTime >= fChargeLevel2Time)
            {
                m_eChargeLevel = CHARGE_LEVEL::LV2;
                m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_Charge2.wav", 0.25f);
            }
            else if (m_eChargeLevel == CHARGE_LEVEL::LV2 && m_fChargeTime >= fChargeLevel3Time)
            {
                m_eChargeLevel = CHARGE_LEVEL::LV3;
                m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_Charge3.wav", 0.25f);
            }
            else if (m_eChargeLevel == CHARGE_LEVEL::LV3 && m_fChargeTime >= fChargeLevel4Time)
            {
                m_eChargeLevel = CHARGE_LEVEL::LV4;
                m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_ChargeBurst.wav", 0.25f);

                CKirby_ToyHammer* pToyHammer = static_cast<CKirby_ToyHammer*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::TOY_HAMMER));

                pToyHammer->BurnHammer(true);
            }

            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_1:
        {
            AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_2:
        {
            AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_3:
        {
            if (AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END))
                break;

            const _float fRatio = pAnimator->Get_Progress();
            if (fRatio >= 0.27f && fRatio <= 0.39f)
            {
                CMovement_Child* pMovement = pKirby->Get_Movement();
                pMovement->Set_MaxHorizontalSpeed(fChargeAttack3MaxHorizontalSpeed);
                MoveLookDir(pKirby, 30.f);
            }

            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_4:
        {
            AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END);
            break;
        }
        case TOY_HAMMER_STATE::WHEELHAMMER:
        {
            if (pAnimator->Is_Finished())
            {
                if (m_bWheelHammerPressing)
                    Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::WHEELHAMMER_FALL);
                else
                    Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::WHEELHAMMER_END);
            }
            break;
        }
        case TOY_HAMMER_STATE::WHEELHAMMER_END:
        {
            AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END);

            const _float fRatio = pAnimator->Get_Progress();
            if (!m_bWheelHammerEndOverlayApplied && fRatio >= 0.4f)
            {
                pAnimator->Set_Mask("HaveHammerWait", "R_ShoulderJ", true, 1.f, 0.2f, 0.f);
                m_bWheelHammerEndOverlayApplied = true;
            }
            break;
        }
        case TOY_HAMMER_STATE::WHEELHAMMER_FALL:
        {
            if(pKirby->Get_Movement()->Is_Grounded())
            {
                Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::TOY_HAMER_STATE_END);
                return;
            }

            if (!m_bWheelHammerPressing)
                Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::TOY_HAMER_STATE_END);

            break;
        }
        case TOY_HAMMER_STATE::REBOUND:
        {
            //Change_ToyHammerState(pKirby, TOY_HAMMER_STATE::TOY_HAMER_STATE_END);
            break;
        }
    }
}

void CKirby_Ability_ToyHammer::Exit_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eState)
{
    switch (eState)
    {
        case TOY_HAMMER_STATE::ATTACK_START:
        case TOY_HAMMER_STATE::ATTACK:
        case TOY_HAMMER_STATE::ATTACK_END:
        case TOY_HAMMER_STATE::ATTACK_MISS:
            break;
        case TOY_HAMMER_STATE::ATTACK_FINAL:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_START:
            break;
        case TOY_HAMMER_STATE::CHARGING:
        {
            Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::NONE);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_1:
        case TOY_HAMMER_STATE::CHARGE_ATTACK_2:
        {
            Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::NONE);
            m_fChargeTime = 0.f;
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_3:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

            Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::NONE);
            m_fChargeTime = 0.f;
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_ATTACK_4:
        {
            Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::NONE);
            m_fChargeTime = 0.f;

            CKirby_ToyHammer* pToyHammer = static_cast<CKirby_ToyHammer*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::TOY_HAMMER));
            pToyHammer->BurnHammer(false);
            break;
        }
        case TOY_HAMMER_STATE::WHEELHAMMER:
        case TOY_HAMMER_STATE::WHEELHAMMER_END:      
        case TOY_HAMMER_STATE::WHEELHAMMER_FALL:
            break;
        case TOY_HAMMER_STATE::REBOUND:
        {
            break;
        }
    }
}

void CKirby_Ability_ToyHammer::Change_ChargeAniState(CKirby* pKirby, CHARGE_ANI_STATE eState)
{
    if (m_eChargeAniState == eState)
        return;

    m_eChargeAniState = eState;

    if (eState == CHARGE_ANI_STATE::NONE)
    {
        Clear_Overlay(pKirby, iChargeOverlaySlot, 0.f);
        return;
    }

    CAnimator::LAYER_PLAY_INFO tLayerInfo{};
    tLayerInfo.iSlot = iChargeOverlaySlot;
    tLayerInfo.tAnim.bRestart = true;
    tLayerInfo.tAnim.fBlend = 0.1f;
    tLayerInfo.tAnim.fSpeed = 2.5f;
    tLayerInfo.fTargetWeight = 1.f;
    tLayerInfo.fWeightBlend = 0.1f;
    tLayerInfo.Roots = { "CenterL" };

    switch (eState)
    {
        case CHARGE_ANI_STATE::WAIT:
            Clear_Overlay(pKirby, iChargeOverlaySlot, 0.1f);
            return;
        case CHARGE_ANI_STATE::MOVE:
            tLayerInfo.tAnim.strAniName = "OnigorosiHammerMove";
            tLayerInfo.tAnim.bLoop = true;
            break;
        case CHARGE_ANI_STATE::JUMP_START:            
            // Overlay Anim Event ¾È ºÒ¸²
            m_pGameInstance_Proxy->Play_SFX(L"HeroHammerBasic_OnigorosiJump.wav", 0.1f);
            tLayerInfo.tAnim.strAniName = "OnigorosiHammerJumpStart";
            tLayerInfo.tAnim.bLoop = false;
            break;
        case CHARGE_ANI_STATE::AIR:
            tLayerInfo.tAnim.strAniName = "OnigorosiHammerJump";
            tLayerInfo.tAnim.bLoop = true;
            break;
        case CHARGE_ANI_STATE::JUMP_END:
            tLayerInfo.tAnim.strAniName = "OnigorosiHammerJumpEnd";
            tLayerInfo.tAnim.bLoop = false;
            break;
    }

    pKirby->Get_Body()->Get_Animator()->Apply_Overlay(tLayerInfo);
}

void CKirby_Ability_ToyHammer::Update_ChargeOverlayAni(CKirby* pKirby, _bool bUseMoveAni, _float fTimeDelta)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    const _bool bIsGrounded = pMovement->Is_Grounded();
    const _bool bShouldPlayMoveAnimation = bUseMoveAni && pKirby->Has_MoveDir();
    const CHARGE_ANI_STATE eNextGroundAnimation = bShouldPlayMoveAnimation ? CHARGE_ANI_STATE::MOVE : CHARGE_ANI_STATE::WAIT;

    switch (m_eChargeAniState)
    {
        case CHARGE_ANI_STATE::JUMP_START:
            if (bIsGrounded)
                Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::JUMP_END);
            else if (pAnimator->Is_Overlay_Finished(iChargeOverlaySlot))
                Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::AIR);
            return;

        case CHARGE_ANI_STATE::AIR:
            if (bIsGrounded)
                Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::JUMP_END);
            return;

        case CHARGE_ANI_STATE::JUMP_END:
            if (!bIsGrounded)
                Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::AIR);
            else if (pAnimator->Is_Overlay_Finished(iChargeOverlaySlot))
                Change_ChargeAniState(pKirby, eNextGroundAnimation);
            return;
    }

    // NONE, WAIT, MOVE
    if (!bIsGrounded)
    {
        Change_ChargeAniState(pKirby, CHARGE_ANI_STATE::AIR);
        return;
    }

    Change_ChargeAniState(pKirby, eNextGroundAnimation);
}

void CKirby_Ability_ToyHammer::MoveLookDir(CKirby* pKirby, _float fSpeed)
{
    _vector vDir = pKirby->Get_Transform()->Get_State(STATE::LOOK);
    vDir = XMVector3Normalize(XMVectorSetY(vDir, 0.f));

    pKirby->Get_Movement()->Add_Velocity(vDir * fSpeed);
}

CKirby_Ability_ToyHammer* CKirby_Ability_ToyHammer::Create()
{
    CKirby_Ability_ToyHammer* pInstance = new CKirby_Ability_ToyHammer();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_ToyHammer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_ToyHammer::Free()
{
    __super::Free();
}