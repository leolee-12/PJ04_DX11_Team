#include "Kirby_Ability_Crash.h"

#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Projectile_Manager.h"
#include "KirbyBomb.h"

#include "Effect_Loader.h"

namespace
{
    constexpr _float fMaxFlameChargeTime = 1.9f;
    constexpr _float fMaxFlameTime = 3.9f;
    constexpr _float fMaxDamageTime = 3.5f;

    constexpr _float fMaxDamageHeight = 5.f;
    constexpr _uint iDamageRotCount = 4;

    constexpr _float fMaxHoverHeight = 1.2f;
    constexpr _float fHoverSearchDistance = 1.3f;

    constexpr _float fCrashMaxHorizontalSpeed = 2.5f;
    constexpr _float fCrashLinearDrag = 8.f;
}

CKirby_Ability_Crash::CKirby_Ability_Crash()
{
}

HRESULT CKirby_Ability_Crash::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"Å©·¡½Ã";

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Crash::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::CRASH;
}

void CKirby_Ability_Crash::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = false;

    m_bKeyUpAttackEnd = false;

    m_fAccFlameChargeTime = 0.f;
    m_fAccFlameTime = 0.f;

    pKirby->Set_UseRenderGroundAlign(false);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_GravityScale(0.3f);
    pMovement->Set_MaxHorizontalSpeed(fCrashMaxHorizontalSpeed);
    pMovement->Set_UseGroundFriction(false);
    pMovement->Set_LinearDrag(fCrashLinearDrag);

    m_eCrashState = CRASH_STATE::CRASH_STATE_END;
    Change_CrashState(pKirby, CRASH_STATE::FLAME_CHARGE_START);
}

void CKirby_Ability_Crash::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_CrashState(pKirby, fTimeDelta);
}

void CKirby_Ability_Crash::Exit_AttackState(CKirby* pKirby)
{
    if (m_eCrashState != CRASH_STATE::CRASH_STATE_END)
        Exit_CrashState(pKirby, m_eCrashState);

    m_eCrashState = CRASH_STATE::CRASH_STATE_END;
    m_bReqEndAttackState = true;

    m_fAccFlameChargeTime = 0.f;
    m_fAccFlameTime = 0.f;
    m_fAccDamageTime = 0.f;

    pKirby->Set_UseRenderGroundAlign(true);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_HoverMode(false);
    pMovement->Set_GravityScale(1.f);
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
    pMovement->Set_UseGroundFriction(true);
    pMovement->Set_LinearDrag(CKirby::s_fLinearDrag);
}

_bool CKirby_Ability_Crash::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

            if (m_eCrashState == CRASH_STATE::DAMAGE || m_eCrashState == CRASH_STATE::FLAME_END)
                return true;

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            pKirby->Add_MoveDir(pMoveCommand->Get_Dir());

            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
            {
                if (pCommand->IsUp())
                {
                    m_bKeyUpAttackEnd = true;
                }
                return true;
            }
        }
    return false;
}

_bool CKirby_Ability_Crash::Enter_Attack_KeyDown(CKirby* pKirby)
{
    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
    return true;
}

_bool CKirby_Ability_Crash::Enter_Attack_KeyPress(CKirby* pKirby)
{
    return true;
}

_bool CKirby_Ability_Crash::Enter_Attack_KeyUp(CKirby* pKirby)
{
    return true;
}

void CKirby_Ability_Crash::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    if (m_eCrashState == CRASH_STATE::DAMAGE)
        return;

    __super::On_Damaged_KirbyState(pKirby, tInfo);
}

void CKirby_Ability_Crash::Change_CrashState(CKirby* pKirby, CRASH_STATE eNext)
{
    if (m_eCrashState == eNext)
        return;

    Exit_CrashState(pKirby, m_eCrashState);

    m_eCrashState = eNext;

    Enter_CrashState(pKirby, m_eCrashState);
}

void CKirby_Ability_Crash::Enter_CrashState(CKirby* pKirby, CRASH_STATE eState)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (eState)
    {
        case CRASH_STATE::FLAME_CHARGE_START:
            m_hChargeSound = m_pGameInstance_Proxy->Play_SFX(L"HeroCrashBasic_Charge1.wav", 0.6f);            

            m_eCrashDamageMode = CRASH_DAMAGE_MODE::DEFAULT;
            pAnimator->Play("FlameChargeStart", false, false, 0.1f, 1.5f);
            break;
        case CRASH_STATE::FLAME_CHARGE:
            m_fAccFlameChargeTime = 0.f;

            if(pKirby->Has_MoveDir() && pKirby->Get_Movement()->Is_Grounded())
            {
                pAnimator->Play("FlameChargeMove", true, false, 0.1f, 1.5f);
                m_bPlayFrameChrageMoveAni = true;
            }
            else
            {
                pAnimator->Play("FlameCharge", true, false, 0.1f, 1.5f);
                m_bPlayFrameChrageMoveAni = false;
            }

            break;

        case CRASH_STATE::FLAME_START:
            pKirby->Get_Movement()->Set_HoverMode(true, fMaxHoverHeight, fHoverSearchDistance);

            m_eCrashDamageMode = CRASH_DAMAGE_MODE::JUMP;
            pAnimator->Play("FlameStart", false, false, 0.1f, 2.5f);
            break;
        case CRASH_STATE::FLAME:
            m_fAccFlameTime = 0.f;
            pAnimator->Play("Flame", true, false, 0.1f, 1.5f);
            break;
        case CRASH_STATE::DAMAGE:
        {            
            m_pGameInstance_Proxy->Set_TimeScale(0.f);

            // Movement
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Stop();
            pMovement->Set_UseGravity(false);
            pMovement->Set_HoverMode(false);

            // Value
            XMStoreFloat3(&m_vDamageStartPos, pKirby->Get_Transform()->Get_State(STATE::POSITION));
            m_iAccDamageRotCount = 0;
            m_fAccDamageTime = 0.f;

            // Sound
            if (m_hChargeSound.Is_Valid())
                m_hChargeSound.Stop();

            if (m_eCrashDamageMode == CRASH_DAMAGE_MODE::DEFAULT)
                m_pGameInstance_Proxy->Play_SFX(L"HeroCrash_Flame1.wav", 0.6f);
            else if (m_fAccFlameTime < fMaxFlameTime)
                m_pGameInstance_Proxy->Play_SFX(L"HeroCrashTime_Attack.wav", 0.6f);
            else
                m_pGameInstance_Proxy->Play_SFX(L"HeroCrashTime_BigAttack.wav", 0.6f);

            // Ani
            if (m_eCrashDamageMode == CRASH_DAMAGE_MODE::DEFAULT)
            {
                pAnimator->Play("FlameStart", false, false, 0.1f, 2.5f);
                CAnimator::ANI_PLAY_INFO tInfo{};
                tInfo.strAniName = "Flame";
                tInfo.bLoop = true;
                tInfo.bRestart = false;
                tInfo.fBlend = 0.1f;
                tInfo.fSpeed = 1.5f;
                pAnimator->Enqueue(tInfo);
            }

            break;
        }
        case CRASH_STATE::FLAME_END:
            pAnimator->Play("FlameEnd", false, false, 0.1f, 1.5f);
            break;
        case CRASH_STATE::CRASH_STATE_END:
            m_bReqEndAttackState = true;
            break;
    }
}

void CKirby_Ability_Crash::Update_CrashState(CKirby* pKirby, _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eCrashState)
    {
        case CRASH_STATE::FLAME_CHARGE_START:
        {
            if (pAnimator->Is_Finished())
            {
                if (m_bKeyUpAttackEnd)
                    Change_CrashState(pKirby, CRASH_STATE::DAMAGE);
                else
                    Change_CrashState(pKirby, CRASH_STATE::FLAME_CHARGE);
            }
            break;
        }
        case CRASH_STATE::FLAME_CHARGE:
        {
            Update_FlameChrageMoveAni(pKirby);

            if (m_bKeyUpAttackEnd)
                Change_CrashState(pKirby, CRASH_STATE::DAMAGE);
            else if (m_fAccFlameChargeTime >= fMaxFlameChargeTime)
                Change_CrashState(pKirby, CRASH_STATE::FLAME_START);
            else
                m_fAccFlameChargeTime += fTimeDelta;
            break;
        }
        case CRASH_STATE::FLAME_START:
        {
            if (m_bKeyUpAttackEnd)
                Change_CrashState(pKirby, CRASH_STATE::DAMAGE);
            else if (pAnimator->Is_Finished())
                Change_CrashState(pKirby, CRASH_STATE::FLAME);
            break;
        }
        case CRASH_STATE::FLAME:
        {
            if (m_bKeyUpAttackEnd || m_fAccFlameTime >= fMaxFlameTime)
                Change_CrashState(pKirby, CRASH_STATE::DAMAGE);
            else
                m_fAccFlameTime += fTimeDelta;
            break;
        }
        case CRASH_STATE::DAMAGE:
        {
            _float fRatio = m_fAccDamageTime / fMaxDamageTime;
            Helper::FloatClamp(fRatio, 0.f, 1.f);
            if (m_eCrashDamageMode == CRASH_DAMAGE_MODE::JUMP)
            {
                _float fMoveRatio{};
                if (fRatio <= 0.5f)
                    fMoveRatio = Helper::FloatSmoothStep(0.f, 0.5f, fRatio);
                else
                    fMoveRatio = 1.f - Helper::FloatSmoothStep(0.5f, 1.f, fRatio);

                const _float fHeight = fMaxDamageHeight * fMoveRatio;
                _vector vPos = XMLoadFloat3(&m_vDamageStartPos);
                vPos = XMVectorSetY(vPos, m_vDamageStartPos.y + fHeight);
                pKirby->Get_Transform()->Set_State(STATE::POSITION, vPos);
                pKirby->Get_Movement()->Sync_To_Controller();

                // Rot
                _float fRotDegree = fmodf(fRatio * static_cast<_float>(iDamageRotCount) * 360.f, 360.f);
                pAnimator->SetBoneRotation("RotL", fRotDegree, XMVectorSet(1.f, 0.f, 0.f, 0.f));
            }

            if (m_fAccDamageTime >= fMaxDamageTime)
            {
                Change_CrashState(pKirby, CRASH_STATE::FLAME_END);
                return;
            }

            m_fAccDamageTime += fTimeDelta;

            break;
        }
        case CRASH_STATE::FLAME_END:
        {
            if (pAnimator->Is_Finished())
                Change_CrashState(pKirby, CRASH_STATE::CRASH_STATE_END);
            break;
        }
    }
}

void CKirby_Ability_Crash::Exit_CrashState(CKirby* pKirby, CRASH_STATE eState)
{
    switch (eState)
    {
        case CRASH_STATE::FLAME_CHARGE_START:
            break;
        case CRASH_STATE::FLAME_CHARGE:
            break;
        case CRASH_STATE::FLAME_START:
            break;
        case CRASH_STATE::FLAME:
            break;
        case CRASH_STATE::DAMAGE:
        {
            m_pGameInstance_Proxy->Set_TimeScale(1.f);
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Stop();
            pMovement->Set_UseGravity(true);

            pKirby->Get_Transform()->Set_State(STATE::POSITION, XMLoadFloat3(&m_vDamageStartPos));
            pMovement->Sync_To_Controller();
            CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
            pAnimator->SetBoneRotation("RotL", 0.f, XMVectorSet(1.f, 0.f, 0.f, 0.f));
            break;
        }
        case CRASH_STATE::FLAME_END:
            break;
    }
}

void CKirby_Ability_Crash::Update_FlameChrageMoveAni(CKirby* pKirby)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    _bool bHasMoveDir = pKirby->Has_MoveDir();

        if (bHasMoveDir && pKirby->Get_Movement()->Is_Grounded() && !m_bPlayFrameChrageMoveAni)
        {
            pAnimator->Play("FlameChargeMove", true, false, 0.1f, 1.5f);
            m_bPlayFrameChrageMoveAni = true;
        }
        else if(!bHasMoveDir && m_bPlayFrameChrageMoveAni)
        {
            pAnimator->Play("FlameCharge", true, false, 0.1f, 1.5f);
            m_bPlayFrameChrageMoveAni = false;
        }
}

CKirby_Ability_Crash* CKirby_Ability_Crash::Create()
{
    CKirby_Ability_Crash* pInstance = new CKirby_Ability_Crash();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Crash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Crash::Free()
{
    __super::Free();
}
