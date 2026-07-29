#include "Kirby_Ability_Crash.h"

#include "Kirby.h"
#include "Kirby_Body.h"

#include "Movement_Child.h"

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
}

void CKirby_Ability_Crash::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Ability_Crash::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Ability_Crash::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
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
            pAnimator->Play("FlameChargeStart", false, true, 0.1f, 1.5f);
            break;
        case CRASH_STATE::FLAME_CHARGE:
            pAnimator->Play("FlameCharge", false, true, 0.1f, 1.5f);
            break;
        case CRASH_STATE::FLAME_START:
            pAnimator->Play("FlameStart", false, true, 0.1f, 1.5f);
            break;
        case CRASH_STATE::FLAME:
            pAnimator->Play("Flame", false, true, 0.1f, 1.5f);
            break;
        case CRASH_STATE::FLAME_END:
            pAnimator->Play("FlameEnd", false, true, 0.1f, 1.5f);
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
            if (pAnimator->Is_Finished())
                Change_CrashState(pKirby, CRASH_STATE::FLAME_CHARGE);
            break;
        case CRASH_STATE::FLAME_CHARGE:
            if (pAnimator->Is_Finished())
                Change_CrashState(pKirby, CRASH_STATE::FLAME_START);
            break;
        case CRASH_STATE::FLAME_START:
            if (pAnimator->Is_Finished())
                Change_CrashState(pKirby, CRASH_STATE::FLAME);
            break;
        case CRASH_STATE::FLAME:
            if (pAnimator->Is_Finished())
                Change_CrashState(pKirby, CRASH_STATE::FLAME_END);
            break;
        case CRASH_STATE::FLAME_END:
            if (pAnimator->Is_Finished())
                Change_CrashState(pKirby, CRASH_STATE::CRASH_STATE_END);
            break;
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
    case CRASH_STATE::FLAME_END:
        break;
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
