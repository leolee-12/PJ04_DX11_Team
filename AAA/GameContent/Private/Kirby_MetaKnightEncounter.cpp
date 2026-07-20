#include "Kirby_MetaKnightEncounter.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_MetaKnightEncounter::CKirby_MetaKnightEncounter()
{
}

HRESULT CKirby_MetaKnightEncounter::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_MetaKnightEncounter::Get_StateType()
{
    return KIRBY_STATE_TYPE::METAKNIGHT_ENCOUNTER;
}

void CKirby_MetaKnightEncounter::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);
}

void CKirby_MetaKnightEncounter::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_MetaKnightEncounter::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_MetaKnightEncounter::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_MetaKnightEncounter::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

CKirby_MetaKnightEncounter* CKirby_MetaKnightEncounter::Create()
{
    CKirby_MetaKnightEncounter* pInstance = new CKirby_MetaKnightEncounter();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_MetaKnightEncounter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_MetaKnightEncounter::Free()
{
    __super::Free();
}