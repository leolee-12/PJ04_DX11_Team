#include "Kirby_Ability_Sleep.h"

#include "Kirby.h"

CKirby_Ability_Sleep::CKirby_Ability_Sleep()
{
}

HRESULT CKirby_Ability_Sleep::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"½½¸³";

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Sleep::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::SLEEP;
}

void CKirby_Ability_Sleep::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = true;
}

void CKirby_Ability_Sleep::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Ability_Sleep::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Ability_Sleep::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    switch (pCommand->GetCommandType())
    {
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
            return true;
    }

    return false;
}

void CKirby_Ability_Sleep::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    __super::On_Damaged_KirbyState(pKirby, tInfo);
}

CKirby_Ability_Sleep* CKirby_Ability_Sleep::Create()
{
    CKirby_Ability_Sleep* pInstance = new CKirby_Ability_Sleep();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Sleep");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Sleep::Free()
{
    __super::Free();
}
