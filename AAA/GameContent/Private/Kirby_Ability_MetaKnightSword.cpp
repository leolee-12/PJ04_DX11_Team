#include "Kirby_Ability_MetaKnightSword.h"

CKirby_Ability_MetaKnightSword::CKirby_Ability_MetaKnightSword()
{
}

HRESULT CKirby_Ability_MetaKnightSword::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"메타 나이트 스워드";

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_MetaKnightSword::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::METAKNIGHT_SWORD;
}

void CKirby_Ability_MetaKnightSword::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = true;
}

void CKirby_Ability_MetaKnightSword::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Ability_MetaKnightSword::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Ability_MetaKnightSword::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
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