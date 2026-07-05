#include "Kirby_QTE_Grabbed.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_QTE_Grabbed::CKirby_QTE_Grabbed()
{
}

HRESULT CKirby_QTE_Grabbed::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_QTE_Grabbed::Get_StateType()
{
    return KIRBY_STATE_TYPE::QTE_GRABBED;
}

void CKirby_QTE_Grabbed::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    pAnimator->Play("Damage", true, false, 0.1f, 2.f);

    pBody->Set_KirbyEye(KIRBY_EYE_STATE::CLOSE);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Set_UseGravity(false);

    pKirby->Set_AbilityPartsActive(pKirby->Get_KirbyAbility()->Get_AbilityType(), false, true);

    Change_QTEGrabbedState(pKirby, QTE_GRABBED_STATE::START);
}

void CKirby_QTE_Grabbed::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
    Update_QTEGrabbedState(pKirby, fTimeDelta);
    pKirby->Update_CutsceneGrabTransform();
}

void CKirby_QTE_Grabbed::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    pKirby->Get_Body()->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);
}

_bool CKirby_QTE_Grabbed::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

void CKirby_QTE_Grabbed::Request_ReleaseGrabState(CKirby* pKirby, GRAB_TYPE eType)
{
    Change_QTEGrabbedState(pKirby, QTE_GRABBED_STATE::ESCAPE);
}

void CKirby_QTE_Grabbed::Change_QTEGrabbedState(CKirby* pKirby, QTE_GRABBED_STATE eNext)
{
    if (m_eQTEGrabbedState == eNext)
        return;

    Exit_QTEGrabbedState(pKirby, m_eQTEGrabbedState);

    m_eQTEGrabbedState = eNext;

    Enter_QTEGrabbedState(pKirby, m_eQTEGrabbedState);
}

void CKirby_QTE_Grabbed::Enter_QTEGrabbedState(CKirby* pKirby, QTE_GRABBED_STATE eState)
{
    switch (m_eQTEGrabbedState)
    {
        case QTE_GRABBED_STATE::START:
        {

            break;
        }
        case QTE_GRABBED_STATE::ESCAPE:
        {      
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_UseGravity(true);
            pMovement->Sync_To_Controller();

            _vector vBackDir = -pKirby->Get_Transform()->Get_State(STATE::LOOK);
            vBackDir = XMVectorSetY(vBackDir, 0.f);
            vBackDir = XMVector3Normalize(vBackDir);
            pKirby->Get_Movement()->Set_Velocity(vBackDir * 15.f);

            pMovement->Set_VelocityY(22.f);

            pKirby->Set_AbilityPartsActive(pKirby->Get_KirbyAbility()->Get_AbilityType(), true, true);
            pKirby->Get_Body()->Get_Animator()->Play("Damage", true, true, 0.1f, 0.8f);
            break;
        }
        case QTE_GRABBED_STATE::GRABBED_STATE_END:
        {      
            Transition_Fall_OR_Wait_OR_Run(pKirby);
            break;
        }
    }
}

void CKirby_QTE_Grabbed::Update_QTEGrabbedState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    switch (m_eQTEGrabbedState)
    {
        case QTE_GRABBED_STATE::START:
        {
            break;
        }
        case QTE_GRABBED_STATE::ESCAPE:
        {
            _float fRatio = pAnimator->Get_Progress();

            const _float fStart = 0.f;
            const _float fMiddle = 0.25f;
            const _float fEnd = 0.57f;

            _float fRotRatio = {};
            if (fRatio >= fMiddle)
                fRotRatio = (fRatio - fMiddle) / (fEnd - fMiddle);
            else if (fRatio >= fStart)
                fRotRatio = fRatio / (fMiddle - fStart);

            Helper::FloatClamp(fRotRatio, 0.f, 1.f);
            _vector vAxis = XMVectorSet(1.f, 0.f, 0.f, 0.f);
            pAnimator->SetBoneRotation("RotL", fRotRatio * 360.f, vAxis);

            if (fRatio >= fEnd)
                Change_QTEGrabbedState(pKirby, QTE_GRABBED_STATE::GRABBED_STATE_END);

            break;
        }
    }
}

void CKirby_QTE_Grabbed::Exit_QTEGrabbedState(CKirby* pKirby, QTE_GRABBED_STATE eState)
{
    switch (m_eQTEGrabbedState)
    {
        case QTE_GRABBED_STATE::START:
        {
            break;
        }
        case QTE_GRABBED_STATE::ESCAPE:
        {
            pKirby->Get_Body()->Get_Animator()->SetBoneRotation("RotL", 0.f, XMVectorSet(1.f, 0.f, 0.f, 0.f));
            break;
        }
        case QTE_GRABBED_STATE::GRABBED_STATE_END:
        {
            break;
        }
    }
}

CKirby_QTE_Grabbed* CKirby_QTE_Grabbed::Create()
{
    CKirby_QTE_Grabbed* pInstance = new CKirby_QTE_Grabbed();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_QTE_Grabbed");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_QTE_Grabbed::Free()
{
    __super::Free();
}
