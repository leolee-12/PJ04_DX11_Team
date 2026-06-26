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

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    pAnimator->Play("DemoAppear2", false, false, 0.1f, 1.f, true);

    pBody->Set_Eye(KIRBY_EYE_STATE::SURPRISED);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Set_UseGravity(false);

    pKirby->OnOffParts(pKirby->Get_KirbyAbility()->Get_AbilityType(), false, true);
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

    pKirby->Get_Body()->Set_Eye(KIRBY_EYE_STATE::IDLE);
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

void CKirby_CutSceneGrabbed::Request_ReleaseGrabState(CKirby* pKirby, GRAB_TYPE eType)
{
    pKirby->OnOffParts(pKirby->Get_KirbyAbility()->Get_AbilityType(), true, true);
    Transition_Fall_OR_Wait_OR_Run(pKirby);
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
