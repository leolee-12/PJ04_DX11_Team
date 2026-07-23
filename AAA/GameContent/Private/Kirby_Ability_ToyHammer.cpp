#include "Kirby_Ability_ToyHammer.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Movement_Child.h"

#include "Effect_Loader.h"

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

    Clear_Overlay(pKirby);

    m_eToyHammerState = TOY_HAMMER_STATE::TOY_HAMER_STATE_END;
    Change_ToyHammerState(pKirby, m_eToyHammerStartState);
    m_eToyHammerStartState = TOY_HAMMER_STATE::TOY_HAMER_STATE_END;
}

void CKirby_Ability_ToyHammer::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_ToyHammerState(pKirby, fTimeDelta);

    m_bIsCharging = false;
}

void CKirby_Ability_ToyHammer::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Ability_ToyHammer::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (pCommand->IsDown())
            {
            }
            else if (pCommand->IsPress())
            {
                m_bIsCharging = true;
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
    {
        m_eToyHammerStartState = TOY_HAMMER_STATE::CHARGE_START;
        pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
    }

    else
    {
        // Á¡ÇÁ °ø°Ý ¹Ì±¸Çö
        //m_eToyHammerStartState = TOY_HAMMER_STATE::CHARGE_START;
        return true;
    }

    //pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

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

    switch (eState)
    {
        case TOY_HAMMER_STATE::ATTACK_START:
        {
            pAnimator->Play("HammerAttackStartToy", false, true, 0.1f, 1.5f);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK:
        {
            pAnimator->Play("HammerAttackToy", false, true, 0.1f, 1.5f);
            break;
        }
        case TOY_HAMMER_STATE::ATTACK_END:
        {
            pAnimator->Play("HammerAttackHitToy", false, true, 0.1f, 1.5f);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_START:
        {
            pAnimator->Play("OnigorosiHammerStart", false, true, 0.1f, 1.5f);
            break;
        }
        case TOY_HAMMER_STATE::CHARGING:
        {
            pAnimator->Play("OnigorosiHammerCharge", true, true, 0.1f, 1.5f);
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
            AniEndChangeState(TOY_HAMMER_STATE::TOY_HAMER_STATE_END);
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_START:
        {
            if(!m_bIsCharging)
                Change_ToyHammerState(pKirby, ATTACK_START);
            else 
                AniEndChangeState(TOY_HAMMER_STATE::CHARGING);

            break;
        }
        case TOY_HAMMER_STATE::CHARGING:
        {
            // Test Out
            if (!m_bIsCharging)
                Change_ToyHammerState(pKirby, TOY_HAMER_STATE_END);
            break;
        }
    }
}

void CKirby_Ability_ToyHammer::Exit_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eState)
{
    switch (eState)
    {
        case TOY_HAMMER_STATE::ATTACK_START:
        {
            break;
        }
        case TOY_HAMMER_STATE::ATTACK:
        {
            break;
        }
        case TOY_HAMMER_STATE::ATTACK_END:
        {
            break;
        }
        case TOY_HAMMER_STATE::CHARGE_START:
        {
            break;
        }
        case TOY_HAMMER_STATE::CHARGING:
        {
            break;
        }
    }
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
