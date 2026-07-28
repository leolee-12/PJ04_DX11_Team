#include "Kirby_Ability_Crash.h"

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
    m_bReqEndAttackState = true;
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
