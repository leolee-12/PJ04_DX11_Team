#include "Kirby_Ability_MetaKnightSword.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Kirby_MetaSword.h"

#include "Movement_Child.h"

#include "Effect_Loader.h"
#include "Effect_Container.h"

namespace
{
    constexpr _float fSpinSlashChargeMaxHorizontalSpeed = CKirby::s_fMaxHorizontalSpeed - 6.f;
    constexpr _float fSlash2MaxHorizontalSpeed = CKirby::s_fMaxHorizontalSpeed - 2.f;
    constexpr _float fSlash3MaxHorizontalSpeed = CKirby::s_fMaxHorizontalSpeed + 5.f;

    constexpr _float fSuperSpinSlashChargeTime = 0.8f;
    constexpr _uint iSuperSpinSlashInitialCount = 7;
    constexpr _uint iSuperSpinSlashJumpRemainCount = 1;

    constexpr _uint iSwordOverlaySlot = 1;
    constexpr const _char* szOverlayMasks[] = { "L_FootJ", "R_FootJ" };

    constexpr _float fSpinSlashFadeOutDuration = 0.2f;
}

CKirby_Ability_MetaKnightSword::CKirby_Ability_MetaKnightSword()
{
}

HRESULT CKirby_Ability_MetaKnightSword::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"메타 나이트 스워드";

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
    Set_FullBodyAni(ABILITY_ANI::SLIDE, "SwordSlide", false, false, 0.1f, 1.5f);

    Set_FullBodyAni(ABILITY_ANI::SLIDE_JUMP_L, "UpwardSlash", false, false, 0.1f, 1.5f);
    Set_FullBodyAni(ABILITY_ANI::SLIDE_JUMP_R, "UpwardSlash", false, false, 0.1f, 1.5f);

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_MetaKnightSword::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::METAKNIGHT_SWORD;
}

void CKirby_Ability_MetaKnightSword::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = false;
    m_iSuperSpinSlashCount = iSuperSpinSlashInitialCount;
    m_bSwordSpinSlashStarted = false;
    m_bMetaSuperSpinSlashStarted = false;

    m_eSwordState = META_SWORD_STATE::SWORD_STATE_END;
    Change_SwordState(pKirby, m_eStartSwordState);
    m_eStartSwordState = META_SWORD_STATE::SWORD_STATE_END;
}

void CKirby_Ability_MetaKnightSword::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_SuperSpinSlashChargeTime(fTimeDelta);
    Update_SwordState(pKirby, fTimeDelta);

    Update_ChargeAnimationOverlay(pKirby);

    if (m_bMoveLock == false)
        pKirby->Add_MoveDir(m_vSwordWishDir);

    m_vSwordWishDir = {};

    m_bSpinSlashCharge = false;

    m_eCurSwordMoveState = META_SWORD_MOVE_STATE::NONE_MOVE;
}

void CKirby_Ability_MetaKnightSword::Exit_AttackState(CKirby* pKirby)
{
    if (m_eSwordState != META_SWORD_STATE::SWORD_STATE_END)
        Exit_SwordState(pKirby, m_eSwordState);

    m_eSwordState = META_SWORD_STATE::SWORD_STATE_END;
    m_eStartSwordState = META_SWORD_STATE::SWORD_STATE_END;

    m_eCurSwordMoveState = META_SWORD_MOVE_STATE::NONE_MOVE;
    m_ePreSwordMoveState = META_SWORD_MOVE_STATE::NONE_MOVE;

    m_bReqEndAttackState = true;
    m_bReserveNextAttack = false;
    m_bSpinSlashCharge = false;
    m_bMoveLock = false;
    m_fAccSuperSpinSlashChargeTime = 0.f;
    m_vSwordWishDir = {};

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Resume_Mask(iSwordOverlaySlot);
    Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);

    pKirby->Set_RotationLock(false);

    FadeOut_SpinSlashEffect(m_pSwordSpinSlash, fSpinSlashFadeOutDuration);
    FadeOut_SpinSlashEffect(m_pMetaSuperSpinSlash, fSpinSlashFadeOutDuration);
    m_bSwordSpinSlashStarted = false;
    m_bMetaSuperSpinSlashStarted = false;
    Effect_Stop(m_pSwordChargeEffect);
    Effect_Stop(m_pSwordSuperChargeEffect);
    Effect_StopImmediately(m_pMetaSwordJumpSpinTrail1);
    Effect_StopImmediately(m_pMetaSwordJumpSpinTrail2);
    Effect_StopImmediately(m_pSwordSpinSlashTrail);

    CKirby_MetaSword* pMetaSword = static_cast<CKirby_MetaSword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::METAKNIGHT_SWORD));
    pMetaSword->End_Hit();
}

_bool CKirby_Ability_MetaKnightSword::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

            m_eCurSwordMoveState = META_SWORD_MOVE_STATE::MOVE_FRONT;

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

            m_eCurSwordMoveState = META_SWORD_MOVE_STATE::MOVE_RIGHT;

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
                if (m_eSwordState == META_SWORD_STATE::SLASH_3)
                    return true;

                if (m_eSwordState == META_SWORD_STATE::JUMP_SLASH)
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

            if (m_eSwordState == META_SWORD_STATE::SUPER_SPIN_SLASH_LOOP && pMovement->Is_Grounded())
            {
                pMovement->Try_Jump();

                if (m_iSuperSpinSlashCount > iSuperSpinSlashJumpRemainCount)
                    m_iSuperSpinSlashCount = iSuperSpinSlashJumpRemainCount;
            }

            return true;
        }
    }

    return false;
}

_bool CKirby_Ability_MetaKnightSword::Enter_Attack_KeyDown(CKirby* pKirby)
{
    if (pKirby->Get_Movement()->Is_Grounded())
        m_eStartSwordState = META_SWORD_STATE::SLASH_1;
    else
        m_eStartSwordState = META_SWORD_STATE::JUMP_SLASH_START;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    return true;
}

_bool CKirby_Ability_MetaKnightSword::Enter_Attack_KeyPress(CKirby* pKirby)
{
    if (pKirby->Get_Movement()->Is_Grounded())
    {
        m_eStartSwordState = META_SWORD_STATE::SPIN_SLASH_CHARGE;
        pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
        m_bSpinSlashCharge = true;
    }

    return true;
}

_bool CKirby_Ability_MetaKnightSword::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // 무시
    return true;
}

void CKirby_Ability_MetaKnightSword::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    __super::On_Damaged_KirbyState(pKirby, tInfo);
}

_bool CKirby_Ability_MetaKnightSword::Handle_BodyAnimEvent(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    CKirby_MetaSword* pMetaSword = static_cast<CKirby_MetaSword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::METAKNIGHT_SWORD));

    if (static_cast<EANIM_EVENT>(e.iEventType) == EANIM_EVENT::Hitbox)
    {
        enum META_SWORD_HIT_PARAM
        {
            SLASH1_H, SLASH_2_H, SLASH_3_H = 5,
            JUMP_SLASH_H,
            SPIN_SLASH, SUPER_SPIN_SLASH,
            UPWARDSLASH
        };

        if (ePhase == ANIM_EVENT_PHASE::BEGIN)
        {
            ATTACK_INFO tAttackInfo{};

            switch (e.iIntParam)
            {
                case COMMON_HIT_PARAM::SLIDE_H:
                {
                    tAttackInfo.fDamage = 10.f;
                    tAttackInfo.fKnockback = 9.5f;
                    tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
                case META_SWORD_HIT_PARAM::SLASH1_H:
                {
                    tAttackInfo.fDamage = 100.f;
                    tAttackInfo.fKnockback = 5.f;
                    tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
                case META_SWORD_HIT_PARAM::SLASH_2_H:
                {
                    tAttackInfo.fDamage = 25.f;
                    tAttackInfo.fKnockback = 5.f;
                    tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
                case META_SWORD_HIT_PARAM::SLASH_3_H:
                {
                    tAttackInfo.fDamage = 200.f;
                    tAttackInfo.fKnockback = 15.f;
                    tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
                case META_SWORD_HIT_PARAM::JUMP_SLASH_H:
                {
                    tAttackInfo.fDamage = 100.f;
                    tAttackInfo.fKnockback = 5.f;
                    tAttackInfo.eHitType = HIT_TYPE::SWORD_DEFAULT;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
                case META_SWORD_HIT_PARAM::SPIN_SLASH:
                {
                    tAttackInfo.fDamage = 100.f;
                    tAttackInfo.fKnockback = 5.f;
                    tAttackInfo.eHitType = HIT_TYPE::SWORD_SPIN;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
                case META_SWORD_HIT_PARAM::SUPER_SPIN_SLASH:
                {
                    tAttackInfo.fDamage = 100.f;
                    tAttackInfo.fKnockback = 15.f;
                    tAttackInfo.eHitType = HIT_TYPE::SWORD_SPIN;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
                case META_SWORD_HIT_PARAM::UPWARDSLASH:
                {
                    tAttackInfo.fDamage = 100.f;
                    tAttackInfo.fKnockback = 5.f;
                    tAttackInfo.eHitType = HIT_TYPE::UPWARD_SLASH;
                    pMetaSword->Begin_Hit(tAttackInfo);
                    return true;
                }
            }
        }

        if (ePhase == ANIM_EVENT_PHASE::END)
        {
            switch (static_cast<META_SWORD_HIT_PARAM>(e.iIntParam))
            {
                case COMMON_HIT_PARAM::SLIDE_H:
                case META_SWORD_HIT_PARAM::SLASH1_H:
                case META_SWORD_HIT_PARAM::SLASH_2_H:
                case META_SWORD_HIT_PARAM::SLASH_3_H:
                case META_SWORD_HIT_PARAM::JUMP_SLASH_H:
                case META_SWORD_HIT_PARAM::SPIN_SLASH:
                case META_SWORD_HIT_PARAM::SUPER_SPIN_SLASH:
                case META_SWORD_HIT_PARAM::UPWARDSLASH:
                    pMetaSword->End_Hit();
                    return true;
            }
        }

        return false;
    }

    return false;
}

void CKirby_Ability_MetaKnightSword::Change_SwordState(CKirby* pKirby, META_SWORD_STATE eNext)
{
    if (m_eSwordState == eNext)
        return;

    Exit_SwordState(pKirby, m_eSwordState);

    m_eSwordState = eNext;

    Enter_SwordState(pKirby, m_eSwordState);
}

void CKirby_Ability_MetaKnightSword::Enter_SwordState(CKirby* pKirby, META_SWORD_STATE eState)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (eState)
    {
        case META_SWORD_STATE::SLASH_1:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);
            pAnimator->Play("Meta_Attack1", false, false, 0.f, 1.5f);

            CEffect_Loader::GetInstance()->Spawn(L"MetaSlash1", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.5f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 8.f),
                pKirby->Get_RenderWorldMatrixPtr());
            break;
        }
        case META_SWORD_STATE::SLASH_1_END:
        {
            pAnimator->Play("SideSlashEnd", false, false, 0.05f, 1.5f);
            break;
        }
        case META_SWORD_STATE::SLASH_2:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);
            pAnimator->Play("Meta_Attack2", false, false, 0.f, 1.5f);

            CEffect_Loader::GetInstance()->Spawn(L"MetaSlash2", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.5f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, -8.f),
                pKirby->Get_RenderWorldMatrixPtr());
            break;
        }
        case META_SWORD_STATE::SLASH_3:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);
            pAnimator->Play("Meta_Attack3", false, false, 0.f, 2.f);
            break;
        }
        case META_SWORD_STATE::JUMP_SLASH_START:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);
            pAnimator->Play("SwordSpinStart", false, false, 0.f, 10.f);
            pKirby->Set_RotationLock(true);
            break;
        }
        case META_SWORD_STATE::JUMP_SLASH:
        {
            pAnimator->Play("SwordSpin", false, false, 0.f, 1.5f);

            CEffect_Loader::GetInstance()->Spawn(L"MetaSwordJumpSpin", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 90.f, 0.f),
                pKirby->Get_RenderWorldMatrixPtr());

            CKirby_MetaSword* pMetaSword = static_cast<CKirby_MetaSword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::METAKNIGHT_SWORD));
            if (pMetaSword != nullptr)
            {
                CEffect_Loader::GetInstance()->Spawn(L"MetaSwordJumpSpinTrail1", pKirby->Get_LevelIndex(),
                    _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                    pMetaSword->Get_CombinedWorldMatrixPtr(), &m_pMetaSwordJumpSpinTrail1);
                CEffect_Loader::GetInstance()->Spawn(L"MetaSwordJumpSpinTrail2", pKirby->Get_LevelIndex(),
                    _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                    pMetaSword->Get_CombinedWorldMatrixPtr(), &m_pMetaSwordJumpSpinTrail2);
            }

            break;
        }
        case META_SWORD_STATE::SPIN_SLASH_CHARGE:
        {
            // Sword Have Clear
            Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);
            pKirby->Set_RotationLock(true);

            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(fSpinSlashChargeMaxHorizontalSpeed);

            pAnimator->Play("SpinSlashCharge", false, false, 0.05f, 2.5f);

            if (m_eCurSwordMoveState == META_SWORD_MOVE_STATE::MOVE_FRONT)
                pAnimator->Set_Mask("ShuffleFront", szOverlayMasks, std::size(szOverlayMasks), true, 1.0f, 0.1f, 0.2f);
            else if (m_eCurSwordMoveState == META_SWORD_MOVE_STATE::MOVE_RIGHT)
                pAnimator->Set_Mask("ShuffleRight", szOverlayMasks, std::size(szOverlayMasks), true, 1.0f, 0.1f, 0.2f);

            CEffect_Loader::GetInstance()->Spawn(L"MetaChargeEffect", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                pKirby->Get_RenderWorldMatrixPtr(), &m_pSwordChargeEffect);

            break;
        }
        case META_SWORD_STATE::SPIN_SLASH:
        {
            // Shuffle Clear
            Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);
            m_bSwordSpinSlashStarted = false;
            pAnimator->Play("SpinSlash", false, false, 0.1f, 2.f);
            break;
        }
        case META_SWORD_STATE::SPIN_SLASH_END:
        {
            pAnimator->Play("SpinSlashEnd", false, false, 0.1f, 3.f);
            break;
        }
        case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(fSpinSlashChargeMaxHorizontalSpeed);
            pAnimator->Play("SuperSpinSlashChargeStart", false, false, 0.1f, 2.f);

            CEffect_Loader::GetInstance()->Spawn(L"MetaSuperChargeEffect", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                pKirby->Get_RenderWorldMatrixPtr(), &m_pSwordSuperChargeEffect);

            break;
        }
        case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(fSpinSlashChargeMaxHorizontalSpeed);
            pAnimator->Play("SuperSpinSlashCharge", true, false, 0.1f, 2.f);
            break;
        }
        case META_SWORD_STATE::SUPER_SPIN_SLASH_START:
        {
            // Shuffle Clear
            Clear_Overlay(pKirby, iSwordOverlaySlot, 0.f);
            m_bMetaSuperSpinSlashStarted = false;
            pAnimator->Play("SuperSpinSlashStart", false, false, 0.1f, 2.f);
            break;
        }
        case META_SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
        {
            pAnimator->Play("SuperSpinSlashLoop", false, true, 0.f, 2.f);
            break;
        }
        case META_SWORD_STATE::SUPER_SPIN_SLASH_END:
        {
            pAnimator->Play("SuperSpinSlashEnd", false, false, 0.f, 3.f);
            break;
        }
        case META_SWORD_STATE::SWORD_STATE_END:
        {
            m_bReqEndAttackState = true;
            break;
        }
    }

    m_bReserveNextAttack = false;
}

void CKirby_Ability_MetaKnightSword::Update_SwordState(CKirby* pKirby, _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    const _float fRatio = pAnimator->Get_Progress();
    const _bool bIsAniFinish = pAnimator->Is_Finished();

    auto AniEndChangeState = [this, pKirby, bIsAniFinish](META_SWORD_STATE eState)
        {
            if (bIsAniFinish)
                Change_SwordState(pKirby, eState);
        };

    switch (m_eSwordState)
    {
        case META_SWORD_STATE::SLASH_1:
            Update_MoveLockByRatio(fRatio, 0.45f, 1.f);

            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    Change_SwordState(pKirby, META_SWORD_STATE::SLASH_2);
                else if (m_bSpinSlashCharge)
                    Change_SwordState(pKirby, META_SWORD_STATE::SPIN_SLASH_CHARGE);
                else
                    Change_SwordState(pKirby, META_SWORD_STATE::SLASH_1_END);
            }
            break;

        case META_SWORD_STATE::SLASH_1_END:
            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    Change_SwordState(pKirby, META_SWORD_STATE::SLASH_2);
                else
                    Change_SwordState(pKirby, META_SWORD_STATE::SWORD_STATE_END);
            }
            break;

        case META_SWORD_STATE::SLASH_2:
            Update_MoveLockByRatio(fRatio, 0.8f, 1.f);
            Update_MaxHorizontalSpeedByRatio(pKirby->Get_Movement(), fRatio, 0.f, 0.8f, fSlash2MaxHorizontalSpeed);

            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    Change_SwordState(pKirby, META_SWORD_STATE::SLASH_3);
                else if (m_bSpinSlashCharge)
                    Change_SwordState(pKirby, META_SWORD_STATE::SPIN_SLASH_CHARGE);
                else
                    Change_SwordState(pKirby, META_SWORD_STATE::SWORD_STATE_END);
            }
            break;

        case META_SWORD_STATE::SLASH_3:
            Update_MoveLockByRatio(fRatio, 0.6f, 1.f);
            Update_MaxHorizontalSpeedByRatio(pKirby->Get_Movement(), fRatio, 0.f, 0.6f, fSlash3MaxHorizontalSpeed);

            if (bIsAniFinish)
            {
                if (m_bSpinSlashCharge)
                    Change_SwordState(pKirby, META_SWORD_STATE::SPIN_SLASH_CHARGE);
                else
                    Change_SwordState(pKirby, META_SWORD_STATE::SWORD_STATE_END);
            }
            break;

        // Jump
        case META_SWORD_STATE::JUMP_SLASH_START:
            AniEndChangeState(META_SWORD_STATE::JUMP_SLASH);
            break;

        case META_SWORD_STATE::JUMP_SLASH:
            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                {
                    if (pKirby->Get_Movement()->Is_Grounded())
                        Change_SwordState(pKirby, META_SWORD_STATE::SLASH_1);
                    else
                        Change_SwordState(pKirby, META_SWORD_STATE::JUMP_SLASH_START);
                }
                else
                {
                    Change_SwordState(pKirby, META_SWORD_STATE::SWORD_STATE_END);
                }
            }
            break;

        // Charge
        case META_SWORD_STATE::SPIN_SLASH_CHARGE:
            if (m_fAccSuperSpinSlashChargeTime >= fSuperSpinSlashChargeTime)
            {
                Change_SwordState(pKirby, META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START);
            }
            else if (m_bSpinSlashCharge == false)
            {
                if (bIsAniFinish)
                    Change_SwordState(pKirby, META_SWORD_STATE::SPIN_SLASH);
                else
                    Change_SwordState(pKirby, META_SWORD_STATE::SWORD_STATE_END);
            }
            break;

        // Spin
        case META_SWORD_STATE::SPIN_SLASH:
        {
            AniEndChangeState(META_SWORD_STATE::SPIN_SLASH_END);

            if (m_bSwordSpinSlashStarted == false && fRatio >= 0.01f)
            {
                m_bSwordSpinSlashStarted = true;
                CEffect_Loader::GetInstance()->Spawn(L"SwordSpinSlash", pKirby->Get_LevelIndex(),
                    _float3(0.f, 1.f, 0.f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 0.f),
                    pKirby->Get_RenderWorldMatrixPtr(), &m_pSwordSpinSlash);

                CKirby_MetaSword* pMetaSword = static_cast<CKirby_MetaSword*>(pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::METAKNIGHT_SWORD));
                if (pMetaSword != nullptr)
                {
                    CEffect_Loader::GetInstance()->Spawn(L"SwordSpinSlashTrail", pKirby->Get_LevelIndex(),
                        _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                        pMetaSword->Get_CombinedWorldMatrixPtr(), &m_pSwordSpinSlashTrail);
                }
            }

            if (fRatio >= 0.78f)
            {
                FadeOut_SpinSlashEffect(m_pSwordSpinSlash, fSpinSlashFadeOutDuration);
                Effect_StopImmediately(m_pSwordSpinSlashTrail);
            }

            break;
        }

        case META_SWORD_STATE::SPIN_SLASH_END:
            Update_MoveLockByRatio(fRatio, 0.f, 0.75f);

            if (fRatio >= 0.75f)
                pKirby->Set_RotationLock(false);

            AniEndChangeState(META_SWORD_STATE::SWORD_STATE_END);
            break;

        // Charge Super
        case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
            AniEndChangeState(META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE);
            break;

        case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
            if (m_bSpinSlashCharge == false)
                Change_SwordState(pKirby, META_SWORD_STATE::SUPER_SPIN_SLASH_START);
            break;

        // Spin Super
        case META_SWORD_STATE::SUPER_SPIN_SLASH_START:
        {
            AniEndChangeState(META_SWORD_STATE::SUPER_SPIN_SLASH_LOOP);

            if (m_bMetaSuperSpinSlashStarted == false && fRatio >= 0.15f)
            {
                m_bMetaSuperSpinSlashStarted = true;
                CEffect_Loader::GetInstance()->Spawn(L"MetaSuperSpinSlash", pKirby->Get_LevelIndex(),
                    _float3(0.f, 1.05f, 0.f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 0.f),
                    pKirby->Get_RenderWorldMatrixPtr(), &m_pMetaSuperSpinSlash);

                CKirby_MetaSword* pMetaSword = static_cast<CKirby_MetaSword*>(
                    pKirby->Find_WeaponPart(COPY_ABILITY_TYPE::METAKNIGHT_SWORD));
                if (pMetaSword != nullptr)
                {
                    CEffect_Loader::GetInstance()->Spawn(L"SwordSpinSlashTrail", pKirby->Get_LevelIndex(),
                        _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
                        pMetaSword->Get_CombinedWorldMatrixPtr(), &m_pSwordSpinSlashTrail);
                }
            }

            break;
        }

        case META_SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
            if (bIsAniFinish)
            {
                if (m_iSuperSpinSlashCount > 0)
                {
                    --m_iSuperSpinSlashCount;
                    pAnimator->Play("SuperSpinSlashLoop", false, true, 0.1f, 2.f);
                }
                else
                {
                    Change_SwordState(pKirby, META_SWORD_STATE::SUPER_SPIN_SLASH_END);
                }
            }
            break;

        case META_SWORD_STATE::SUPER_SPIN_SLASH_END:
            Update_MoveLockByRatio(fRatio, 0.f, 0.75f);

            if (fRatio >= 0.75f)
                pKirby->Set_RotationLock(false);

            AniEndChangeState(META_SWORD_STATE::SWORD_STATE_END);
            break;
    }
}

void CKirby_Ability_MetaKnightSword::Exit_SwordState(CKirby* pKirby, META_SWORD_STATE eState)
{
    switch (eState)
    {
        case META_SWORD_STATE::SLASH_1:
            m_bMoveLock = false;
            break;
        case META_SWORD_STATE::SLASH_1_END:
            break;
        case META_SWORD_STATE::SLASH_2:
        case META_SWORD_STATE::SLASH_3:
        {
            m_bMoveLock = false;

            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            break;
        }
        case META_SWORD_STATE::JUMP_SLASH_START:
            break;
        case META_SWORD_STATE::JUMP_SLASH:
            pKirby->Set_RotationLock(false);
            Effect_StopImmediately(m_pMetaSwordJumpSpinTrail1);
            Effect_StopImmediately(m_pMetaSwordJumpSpinTrail2);
            break;
        case META_SWORD_STATE::SPIN_SLASH_CHARGE:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

            Effect_Stop(m_pSwordChargeEffect);
            break;
        }
        case META_SWORD_STATE::SPIN_SLASH:
            break;
        case META_SWORD_STATE::SPIN_SLASH_END:
            m_bMoveLock = false;
            break;
        case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            break;
        }
        case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

            Effect_Stop(m_pSwordSuperChargeEffect);
            break;
        }
        case META_SWORD_STATE::SUPER_SPIN_SLASH_START:
            break;
        case META_SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
            FadeOut_SpinSlashEffect(m_pMetaSuperSpinSlash, fSpinSlashFadeOutDuration);
            Effect_StopImmediately(m_pSwordSpinSlashTrail);
            break;
        case META_SWORD_STATE::SUPER_SPIN_SLASH_END:
            m_bMoveLock = false;
            break;
        case META_SWORD_STATE::SWORD_STATE_END:
            break;
    }
}

void CKirby_Ability_MetaKnightSword::Update_SuperSpinSlashChargeTime(_float fTimeDelta)
{
    const _bool bCanAccumulateChargeTime = m_eSwordState == META_SWORD_STATE::SPIN_SLASH_CHARGE && m_bSpinSlashCharge;

    if (bCanAccumulateChargeTime)
    {
        m_fAccSuperSpinSlashChargeTime += fTimeDelta;
        return;
    }

    m_fAccSuperSpinSlashChargeTime = 0.f;
}

void CKirby_Ability_MetaKnightSword::Update_ChargeAnimationOverlay(CKirby* pKirby)
{
    if (m_eCurSwordMoveState != m_ePreSwordMoveState)
    {
        CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

        switch (m_eSwordState)
        {
            case META_SWORD_STATE::SPIN_SLASH_CHARGE:
            case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
            case META_SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
            {
                const _char* szMoveAnimation = nullptr;

                switch (m_eCurSwordMoveState)
                {
                    case META_SWORD_MOVE_STATE::NONE_MOVE:
                        pAnimator->Pause_Mask(iSwordOverlaySlot);
                        break;
                    case META_SWORD_MOVE_STATE::MOVE_FRONT:
                        szMoveAnimation = "ShuffleFront";
                        break;
                    case META_SWORD_MOVE_STATE::MOVE_RIGHT:
                        szMoveAnimation = "ShuffleRight";
                        break;
                }

                if (szMoveAnimation != nullptr)
                {
                    pAnimator->Set_Mask(szMoveAnimation, szOverlayMasks, std::size(szOverlayMasks), true, 1.f, 0.1f, 0.2f);
                    pAnimator->Resume_Mask(iSwordOverlaySlot);
                }
                break;
            }
        }

        m_ePreSwordMoveState = m_eCurSwordMoveState;
    }
}

void CKirby_Ability_MetaKnightSword::Update_MoveLockByRatio(_float fRatio, _float fRatioStart, _float fRatioEnd)
{
    const _bool bIsInMoveLockRange = fRatio >= fRatioStart && fRatio < fRatioEnd;

    m_bMoveLock = bIsInMoveLockRange;
}

void CKirby_Ability_MetaKnightSword::Update_MaxHorizontalSpeedByRatio(CMovement_Child* pMovement, _float fRatio, _float fRatioStart, _float fRatioEnd, _float fSpeed)
{
    const _bool bIsInSpeedRange = fRatio >= fRatioStart && fRatio < fRatioEnd;

    if (bIsInSpeedRange)
    {
        pMovement->Set_MaxHorizontalSpeed(fSpeed);
        return;
    }

    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
}

void CKirby_Ability_MetaKnightSword::FadeOut_SpinSlashEffect(CEffect_Container*& pEffectContainer, _float fFadeOutDuration)
{
    if (pEffectContainer != nullptr)
    {
        pEffectContainer->Start_FadeOut(fFadeOutDuration);
        pEffectContainer = nullptr;
    }
}

CKirby_Ability_MetaKnightSword* CKirby_Ability_MetaKnightSword::Create()
{
    CKirby_Ability_MetaKnightSword* pInstance = new CKirby_Ability_MetaKnightSword();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_MetaKnightSword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_MetaKnightSword::Free()
{
    __super::Free();
}
