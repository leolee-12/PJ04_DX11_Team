#include "Kirby_Ability_Normal.h"

#include "GameInstance.h"

#include "Kirby.h"

CKirby_Ability_Normal::CKirby_Ability_Normal()
{
}

HRESULT CKirby_Ability_Normal::Initialize()
{

    return S_OK;
}

KIRBY_ABILITY_TYPE CKirby_Ability_Normal::Get_AbilityType()
{
    return KIRBY_ABILITY_TYPE::NORMAL;
}

void CKirby_Ability_Normal::On_Attack(CKirby* pKirby)
{

}

CKirby_Ability_Normal* CKirby_Ability_Normal::Create()
{
    CKirby_Ability_Normal* pInstance = new CKirby_Ability_Normal();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Normal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Normal::Free()
{
    __super::Free();
}
