#include "Kirby_Ability_ToyHammer.h"

CKirby_Ability_ToyHammer::CKirby_Ability_ToyHammer()
{
}

HRESULT CKirby_Ability_ToyHammer::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"Toy Hammer";

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_ToyHammer::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::TOY_HAMMER;
}

void CKirby_Ability_ToyHammer::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = true;
}

void CKirby_Ability_ToyHammer::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Ability_ToyHammer::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Ability_ToyHammer::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
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
