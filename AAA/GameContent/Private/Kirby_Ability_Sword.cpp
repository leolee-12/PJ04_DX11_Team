#include "Kirby_Ability_Sword.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Ability_Sword::CKirby_Ability_Sword()
{
}

HRESULT CKirby_Ability_Sword::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_fSuperSpinSlashChargeTime = 0.8f;

    return S_OK;
}

KIRBY_ABILITY_TYPE CKirby_Ability_Sword::Get_AbilityType()
{
    return KIRBY_ABILITY_TYPE::SWORD;
}

void CKirby_Ability_Sword::Enter_Ability(CKirby* pKirby)
{
    m_bReqEndAttackState = false;
    m_iSuperSpinSlashCount = 7;
    m_bForceEnterSwordAni = false;

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Eye(KIRBY_EYE_STATE::ANGRY);
}

ABILITY_UPDATE_RESULT CKirby_Ability_Sword::Update_Ability(CKirby* pKirby, _float fTimeDelta)
{
    Update_ChargeTime(fTimeDelta);
    
    CMovement_Child* pMovementCom = pKirby->Get_Movement();

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    Update_SwordState(pAnimator, pMovementCom, fTimeDelta);
    Enter_SwordAni(pAnimator, fTimeDelta);
  
    Check_EndAttackState(pAnimator, fTimeDelta);

    m_bSpinSlashCharge = false;

    return ABILITY_UPDATE_RESULT::NONE;
}

void CKirby_Ability_Sword::Exit_Ability(CKirby* pKirby)
{
    m_eCurSwordState = SWORD_STATE::NONE;
    m_ePreSwordState = SWORD_STATE::NONE;

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);
}

_bool CKirby_Ability_Sword::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Attack Down
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (pCommand->IsDown())
            {
                if (m_eCurSwordState == SWORD_STATE::SLASH_3)
                    return true;

                if (m_eCurSwordState == SWORD_STATE::JUMP_SLASH)
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
    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    if(pKirby->Get_Movement()->Is_Grounded() == true)
        m_eCurSwordState = SWORD_STATE::SLASH_1;
    else 
        m_eCurSwordState = SWORD_STATE::JUMP_SLASH_START;

    return true;
}

_bool CKirby_Ability_Sword::Enter_Attack_KeyPress(CKirby* pKirby)
{
    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
    m_eCurSwordState = SWORD_STATE::SPIN_SLASH_CHARGE;

    m_bSpinSlashCharge = true;

    return true;
}

_bool CKirby_Ability_Sword::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // 무시
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

void CKirby_Ability_Sword::Update_SwordState(CAnimator* pAnimator, CMovement_Child* pMovement, _float fTimeDelta)
{
    _bool bIsAniFinish = pAnimator->Is_Finished();


    switch (m_eCurSwordState)
    {
        // Charge
        case SWORD_STATE::SPIN_SLASH_CHARGE:
        {
            if(m_fAccSuperSpinSlashChargeTime >= m_fSuperSpinSlashChargeTime)
                m_eCurSwordState = SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START;
            else if (bIsAniFinish && m_bSpinSlashCharge == false)
                m_eCurSwordState = SWORD_STATE::SPIN_SLASH;
            else if (!bIsAniFinish && m_bSpinSlashCharge == false)
                m_eCurSwordState = SWORD_STATE::NONE;

            break;
        }
        // Super Charge
        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
        {
            if(bIsAniFinish)
                m_eCurSwordState = SWORD_STATE::SUPER_SPIN_SLASH_CHARGE;

            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
        {
            // Super Spin 시작
            if (m_bSpinSlashCharge == false)
                m_eCurSwordState = SWORD_STATE::SUPER_SPIN_SLASH_START;

            break;
        }

        // Spin
        case SWORD_STATE::SPIN_SLASH:
        {
            if (bIsAniFinish)
                m_eCurSwordState = SWORD_STATE::SPIN_SLASH_END;

            break;
        }
        case SWORD_STATE::SPIN_SLASH_END:
        {
            if (bIsAniFinish == true)
                m_eCurSwordState = SWORD_STATE::NONE;

            break;
        }

        // Super Spin
        case SWORD_STATE::SUPER_SPIN_SLASH_START:
        {
            // Super Spin Loop
            if (bIsAniFinish == true)
                m_eCurSwordState = SWORD_STATE::SUPER_SPIN_SLASH_LOOP;

            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
        {
            // Super Spin 끝
            if (bIsAniFinish == true)
            {
                if (m_iSuperSpinSlashCount > 0)
                {
                    --m_iSuperSpinSlashCount;
                    m_bForceEnterSwordAni = true;
                }
                else
                {
                    m_eCurSwordState = SWORD_STATE::SUPER_SPIN_SLASH_END;
                }
            }

            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_END:
        {
            if (bIsAniFinish == true)
                m_eCurSwordState = SWORD_STATE::NONE;

            break;
        }


        // Ground
        case SWORD_STATE::SLASH_1:
        {
            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    m_eCurSwordState = SWORD_STATE::SLASH_2;
                else if (m_bSpinSlashCharge)
                    m_eCurSwordState = SWORD_STATE::SPIN_SLASH_CHARGE;
                else
                    m_eCurSwordState = SWORD_STATE::SLASH_1_END;
            }

            break;
        }
        case SWORD_STATE::SLASH_1_END:
        {
            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    m_eCurSwordState = SWORD_STATE::SLASH_2;
                else
                    m_eCurSwordState = SWORD_STATE::NONE;
            }
  
            break;
        }

        case SWORD_STATE::SLASH_2:
        {
            if (bIsAniFinish)
            {
                if (m_bReserveNextAttack)
                    m_eCurSwordState = SWORD_STATE::SLASH_3;
                else if (m_bSpinSlashCharge)
                    m_eCurSwordState = SWORD_STATE::SPIN_SLASH_CHARGE;
            }

            break;
        }

        case SWORD_STATE::SLASH_3:
        {
            if (bIsAniFinish)
            {
                if (m_bSpinSlashCharge)
                    m_eCurSwordState = SWORD_STATE::SPIN_SLASH_CHARGE;
                else
                    m_eCurSwordState = SWORD_STATE::NONE;
            }
            break;
        }

        // Jump
        case SWORD_STATE::JUMP_SLASH_START:
        {
            if (bIsAniFinish)
                m_eCurSwordState = SWORD_STATE::JUMP_SLASH;

            break;
        }

        case SWORD_STATE::JUMP_SLASH:
        {
            if (bIsAniFinish && m_bReserveNextAttack)
            {
                if(pMovement->Is_Grounded() == true)
                    m_eCurSwordState = SWORD_STATE::SLASH_1;
                else
                    m_eCurSwordState = SWORD_STATE::JUMP_SLASH_START;
            }
            else if (bIsAniFinish)
            {
                m_eCurSwordState = SWORD_STATE::NONE;
            }

            break;
        }
    }
}

void CKirby_Ability_Sword::Enter_SwordAni(CAnimator* pAnimator, _float fTimeDelta)
{
    if (m_eCurSwordState != m_ePreSwordState || m_bForceEnterSwordAni == true)
    {
        switch (m_eCurSwordState)
        {
            // Charge
            case SWORD_STATE::SPIN_SLASH_CHARGE:
                pAnimator->Play("SpinSlashCharge", false, false, 0.05f, 2.5f);
                break;

            // Spin
            case SWORD_STATE::SPIN_SLASH:
                pAnimator->Play("SpinSlash", false, false, 0.1f, 2.f);
                break;
            case SWORD_STATE::SPIN_SLASH_END:
                pAnimator->Play("SpinSlashEnd", false, false, 0.1f, 2.f);
                break;

            // Super Charge
            case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
                pAnimator->Play("SuperSpinSlashChargeStart", false, false, 0.1f, 2.f);
                break;
            case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
                pAnimator->Play("SuperSpinSlashCharge", true, false, 0.1f, 2.f);
                break;

            // Super Spin
            case SWORD_STATE::SUPER_SPIN_SLASH_START:
                pAnimator->Play("SuperSpinSlashStart", false, false, 0.1f, 2.f);
                break;
            case SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
                pAnimator->Play("SuperSpinSlashLoop", false, true, 0.1f, 2.f);
                break;
            case SWORD_STATE::SUPER_SPIN_SLASH_END:
                pAnimator->Play("SuperSpinSlashEnd", false, false, 0.1f, 2.f);
                break;

            // Ground
            case SWORD_STATE::SLASH_1:
                pAnimator->Play("SideSlash", false, false, 0.1f, 1.5);
                break;
            case SWORD_STATE::SLASH_1_END:
                pAnimator->Play("SideSlashEnd", false, false, 0.1f, 2.f);
                break;
            case SWORD_STATE::SLASH_2:
                pAnimator->Play("MultiswordAttack", false, false, 0.1f, 2.f);
                break;
            case SWORD_STATE::SLASH_3:
                pAnimator->Play("DecisiveSlash", false, false, 0.1f, 2.f);
                break;

                // Jump
            case SWORD_STATE::JUMP_SLASH_START:
                pAnimator->Play("SwordSpinStart", false, false, 0.1f, 1.5f);
                break;
            case SWORD_STATE::JUMP_SLASH:
                pAnimator->Play("SwordSpin", false, false, 0.1f, 1.5f);
                break;
        }

        m_ePreSwordState = m_eCurSwordState;
        m_bReserveNextAttack = false;
        m_bForceEnterSwordAni = false;
    }
}

void CKirby_Ability_Sword::Check_EndAttackState(CAnimator* pAnimator, _float fTimeDelta)
{
    _bool bIsAniFinish = pAnimator->Is_Finished();

    switch (m_eCurSwordState)
    {
        case SWORD_STATE::NONE:
        {
            m_bReqEndAttackState = true;
            break;
        }

        case SWORD_STATE::SLASH_1:
        {
            break;
        }

        case SWORD_STATE::SLASH_1_END:
        case SWORD_STATE::SLASH_2:
        case SWORD_STATE::SLASH_3:
        {
            if (bIsAniFinish == true)
                m_bReqEndAttackState = true;

            break;
        }

        case SWORD_STATE::JUMP_SLASH_START:
        {
            break;
        }

        case SWORD_STATE::JUMP_SLASH:
        {
            if (bIsAniFinish == true)
                m_bReqEndAttackState = true;

            break;
        }

        // Charge
        case SWORD_STATE::SPIN_SLASH_CHARGE:
        {
            break;
        }

        // Spin
        case SWORD_STATE::SPIN_SLASH:
        {
            break;
        }

        case SWORD_STATE::SPIN_SLASH_END:
        {
            if (bIsAniFinish == true)
                m_bReqEndAttackState = true;

            break;
        }

        // Super Charge
        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE_START:
        {
            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_CHARGE:
        {
            break;
        }

        // Super Spin
        case SWORD_STATE::SUPER_SPIN_SLASH_START:
        {
            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_LOOP:
        {
            break;
        }
        case SWORD_STATE::SUPER_SPIN_SLASH_END:
        {
            if (bIsAniFinish == true)
                m_bReqEndAttackState = true;
            break;
        }
    }
}

void CKirby_Ability_Sword::Update_ChargeTime(_float fTimeDelta)
{
    if (m_eCurSwordState == SWORD_STATE::SPIN_SLASH_CHARGE && m_bSpinSlashCharge == true)
    {
        m_fAccSuperSpinSlashChargeTime += fTimeDelta;
    }
    else
    {
        m_fAccSuperSpinSlashChargeTime = 0.f;
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