#include "Kirby_CutSceneGrab.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_CutSceneGrab::CKirby_CutSceneGrab()
{
}

HRESULT CKirby_CutSceneGrab::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_CutSceneGrab::Get_StateType()
{
    return KIRBY_STATE_TYPE::CUTSCENEGRAB;
}

void CKirby_CutSceneGrab::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);
}

void CKirby_CutSceneGrab::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_CutSceneGrab::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_CutSceneGrab::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    //switch (eCommandType)
    //{
    //    default:
    //        break;
    //}

    return false;
}

CKirby_CutSceneGrab* CKirby_CutSceneGrab::Create()
{
    CKirby_CutSceneGrab* pInstance = new CKirby_CutSceneGrab();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_CutSceneGrab");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_CutSceneGrab::Free()
{
    __super::Free();
}
