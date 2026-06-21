#include "Kirby_Ability_Sword.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Kirby_Sword.h"

CKirby_Ability_Sword::CKirby_Ability_Sword()
{
}

HRESULT CKirby_Ability_Sword::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    Set_OverlayAni(ABILITY_ANI::WAIT, "Wait", "Sword_HaveSwordWait", "R_ShoulderJ",
        true, false, 1.8f, 0.1f,
        true, false, 1.8f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::RUN, "Run", "HaveSwordMove", "R_ShoulderJ",
        true, false, 2.2f, 0.1f,
        true, false, 2.2f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::FALL, "Fall", "HaveSwordMove", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::LANDING, "Landing", "HaveSwordMove", "R_ShoulderJ",
        false, false, 1.f, 0.05f,
        false, false, 1.f, 0.05f);

    // Jump
    Set_OverlayAni(ABILITY_ANI::JUMP_L, "JumpL", "HaveSwordMove", "R_ShoulderJ",
        false, false, 5.f, 0.1f,
        false, false, 5.f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_R, "JumpR", "HaveSwordMove", "R_ShoulderJ",
        false, false, 5.f, 0.1f,
        false, false, 5.f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_END_L, "JumpEndL", "HaveSwordMove", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::JUMP_END_R, "JumpEndR", "HaveSwordMove", "R_ShoulderJ",
        false, false, 2.f, 0.1f,
        false, false, 2.f, 0.1f);

    // Hovering
    Set_OverlayAni(ABILITY_ANI::FLIGHT_START, "FlightStart", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, false, 2.25f, 0.1f,
        false, false, 2.25f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT, "Flight", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, true, 2.f, 0.1f,
        false, true, 2.f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT_FALL, "FlightFall", "HaveSwordWaitFlight", "R_ShoulderJ",
        true, false, 2.f, 0.1f,
        true, false, 2.f, 0.1f);
    Set_OverlayAni(ABILITY_ANI::FLIGHT_LANDING, "FlightLanding", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, false, 2.5f, 0.1f,
        false, false, 2.5f, 0.1f);

    Set_OverlayAni(ABILITY_ANI::AIR_BALL, "AirBall", "HaveSwordWaitFlight", "R_ShoulderJ",
        false, false, 5.f, 0.05f,
        false, false, 5.f, 0.05f);


    m_fSuperSpinSlashChargeTime = 0.8f;

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Sword::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::SWORD;
}

void CKirby_Ability_Sword::Enter_Ability(CKirby* pKirby)
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

    m_eCurSwordMoveState = SWORD_MOVE_STATE::MOVE_STATE_NONE;
    m_ePreSwordMoveState = SWORD_MOVE_STATE::MOVE_STATE_NONE;

    m_bReqEndAttackState = false;
    m_bReserveNextAttack = false;
    m_bSpinSlashCharge = false;
    m_bMoveLock = false;

    m_fAccSuperSpinSlashChargeTime = 0.f;
    m_iSuperSpinSlashCount = 7;

    ZeroMemory(&m_vSwordWishDir, sizeof(m_vSwordWishDir));

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Eye(KIRBY_EYE_STATE::ANGRY);

    Change_SwordState(pKirby, eStartState);
}

ABILITY_UPDATE_RESULT CKirby_Ability_Sword::Update_Ability(CKirby* pKirby, _float fTimeDelta)
{
    if (Has_SwordMoveDir())
        m_eCurSwordMoveState = SWORD_MOVE_STATE::MOVE;
    else
        m_eCurSwordMoveState = SWORD_MOVE_STATE::NONE_MOVE;

    Update_ChargeTime(fTimeDelta);
    Update_SwordState(pKirby, fTimeDelta);

    ChargeAnimationOverlay(pKirby);

    if (m_bMoveLock == false)
        pKirby->Add_MoveDir(m_vSwordWishDir);

    ZeroMemory(&m_vSwordWishDir, sizeof(m_vSwordWishDir));

    m_bSpinSlashCharge = false;

    return ABILITY_UPDATE_RESULT::NONE;
}

void CKirby_Ability_Sword::Exit_Ability(CKirby* pKirby)
{
    Change_SwordState(pKirby, SWORD_STATE::END);

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Clear_Mask(0.1f);

    m_eSwordState = SWORD_STATE::END;

    m_eCurSwordMoveState = SWORD_MOVE_STATE::MOVE_STATE_NONE;
    m_ePreSwordMoveState = SWORD_MOVE_STATE::MOVE_STATE_NONE;

    m_bReqEndAttackState = true;
    m_bReserveNextAttack = false;
    m_bSpinSlashCharge = false;
    m_bMoveLock = false;

    m_fAccSuperSpinSlashChargeTime = 0.f;

    ZeroMemory(&m_vSwordWishDir, sizeof(m_vSwordWishDir));

    pKirby->Set_RotationLock(false);
    pKirby->Get_Movement()->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);

    CKirby_Sword* pSword = static_cast<CKirby_Sword*>(pKirby->Find_OnOffPart(CKirby_Sword::Kirby_PartTag));
    if (pSword)
    {
        //pSword->Set_HitBox(false);
        pSword->Reset_HitList();
    }
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
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

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
    // ¹«½Ã
    return true;
}

_bool CKirby_Ability_Sword::Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation)
{
    switch (eAttackLocation)
    {
        case KIRBY_ATTACK_LOCATION::GROUND:     return true;
        case KIRBY_ATTACK_LOCATION::AIR:        return true;
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
    CKirby_Sword* pSword = static_cast<CKirby_Sword*>(pKirby->Find_OnOffPart(CKirby_Sword::Kirby_PartTag));

    auto BeginHit = [pSword](_bool bReset)
        {
            if (pSword == nullptr)
                return;

            if (bReset)
                pSword->Reset_HitList();

            pSword->Set_HitBox(true);
        };

    auto EndHit = [pSword]()
        {
            if (pSword == nullptr)
                return;

            pSword->Set_HitBox(false);
            pSword->Reset_HitList();
        };

    m_bReqEndAttackState = false;

    switch (eState)
    {
    case SWORD_STATE::END:
        EndHit();
        m_bReqEndAttackState = true;
        m_bMoveLock = false;
        pKirby->Set_RotationLock(false);
        pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
        break;

    case SWORD_STATE::SLASH_1:
        BeginHit(true);
        pAnimator->Play("SideSlash", false, false, 0.1f, 1.5f);
        break;

    case SWORD_STATE::SLASH_1_END:
        EndHit();
        pAnimator->Play("SideSlashEnd", false, false, 0.1f, 2.f);
        break;

    case SWORD_STATE::SLASH_2:
        BeginHit(true);
        pAnimator->Play("MultiswordAttack", false, false, 0.1f, 2.f);
        break;

    case SWORD_STATE::SLASH_3:
        BeginHit(true);
        pAnimator->Play("DecisiveSlash", false, false, 0.1f, 2.f);
        break;

    case SWORD_STATE::JUMP_SLASH_START:
        BeginHit(true);
        pAnimator->Play("SwordSpinStart", false, false, 0.05f, 9.f);
        break;

    case SWORD_STATE::JUMP_SLASH:
        BeginHit(true);
        pAnimator->Play("SwordSpin", false, false, 0.0f, 1.5f);
        break;

    case SWORD_STATE::SPIN_SLASH_CHARGE:
        EndHit();
        pKirby->Set_RotationLock(true);
        pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed - 6.f);
        pAnimator->Play("SpinSlashCharge", false, false, 0.05f, 2.5f);

        if (m_eCurSwordMoveState == SWORD_MOVE_STATE::MOVE)
            pAnimator->Set_Mask("ShuffleFront", OverlayMasks, std::size(OverlayMasks), true, 1.0f, 0.1f);
        break;

    case SWORD_STATE::SPIN_SLASH:
        BeginHit(true);
        pKirby->Set_RotationLock(true);
        pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

        pAnimator->Play("SpinSlash", false, false, 0.1f, 2.f, false);
        pAnimator->Clear_Mask(0.1f);
        break;

    case SWORD_STATE::SPIN_SLASH_END:
        BeginHit(false);
        pKirby->Set_RotationLock(true);
        pAnimator->Play("SpinSlashEnd", false, false, 0.1f, 2.5f);
        break;

    case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
        EndHit();
        pKirby->Set_RotationLock(true);
        pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed - 6.f);
        pAnimator->Play("SuperSpinSlashChargeStart", false, false, 0.1f, 2.f, false);
        break;

    case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
        EndHit();
        pKirby->Set_RotationLock(true);
        pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed - 6.f);
        pAnimator->Play("SuperSpinSlashCharge", true, false, 0.1f, 2.f, false);
        break;

    case SWORD_STATE::SUPER_SPIN_SLASH_START:
        BeginHit(true);
        pKirby->Set_RotationLock(true);
        pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

        pAnimator->Play("SuperSpinSlashStart", false, false, 0.1f, 2.f, false);
        pAnimator->Clear_Mask(0.1f);
        break;

    case SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
        BeginHit(false);
        pKirby->Set_RotationLock(true);
        pAnimator->Play("SuperSpinSlashLoop", false, true, 0.1f, 2.f);
        break;

    case SWORD_STATE::SUPER_SPIN_SLASH_END:
        BeginHit(false);
        pKirby->Set_RotationLock(true);
        pAnimator->Play("SuperSpinSlashEnd", false, false, 0.1f, 2.5f);
        break;
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
            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::SPIN_SLASH_END);
            break;

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
            if (bIsAniFinish)
                Change_SwordState(pKirby, SWORD_STATE::SUPER_SPIN_SLASH_LOOP);
            break;

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

    pKirby->Set_RotationLock(false);
    pKirby->Get_Movement()->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eSwordState)
    {
        case END:
        case SLASH_1:
        case SLASH_1_END:
        case SLASH_2:
        case SLASH_3:

        case JUMP_SLASH_START:
        case JUMP_SLASH:

        case SPIN_SLASH_CHARGE:
        case SPIN_SLASH:
        case SPIN_SLASH_END:

        case SUPER_SPIN_SLASH_CHARGE_START:
        case SUPER_SPIN_SLASH_CHARGE:
        case SUPER_SPIN_SLASH_START:
        case SUPER_SPIN_SLASH_LOOP:
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
                    pAnimator->Clear_Mask(0.1f);
                }
                else if (m_eCurSwordMoveState == SWORD_MOVE_STATE::MOVE)
                {
                    pAnimator->Set_Mask("ShuffleFront", OverlayMasks, std::size(OverlayMasks), true, 1.0f, 0.1f);
                }
                break;
        }

        m_ePreSwordMoveState = m_eCurSwordMoveState;
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