#include "Kirby_CutSceneGrabbed.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_CutSceneGrabbed::CKirby_CutSceneGrabbed()
{
}

HRESULT CKirby_CutSceneGrabbed::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_CutSceneGrabbed::Get_StateType()
{
    return KIRBY_STATE_TYPE::CUTSCENE_GRABBED;
}

void CKirby_CutSceneGrabbed::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Set_UseGravity(false);
}

void CKirby_CutSceneGrabbed::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    pKirby->Update_CutsceneGrabTransform();
}

void CKirby_CutSceneGrabbed::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_UseGravity(true);
    pMovement->Sync_To_Controller();
}

_bool CKirby_CutSceneGrabbed::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

CKirby_CutSceneGrabbed* CKirby_CutSceneGrabbed::Create()
{
    CKirby_CutSceneGrabbed* pInstance = new CKirby_CutSceneGrabbed();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_CutSceneGrabbed");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_CutSceneGrabbed::Free()
{
    __super::Free();
}
