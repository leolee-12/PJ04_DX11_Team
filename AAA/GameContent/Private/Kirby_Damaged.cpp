#include "Kirby_Damaged.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_Damaged::CKirby_Damaged()
{
}

HRESULT CKirby_Damaged::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Damaged::Get_StateType()
{
    return KIRBY_STATE_TYPE::DAMAGED;
}

void CKirby_Damaged::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);
    pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::DAMAGED);

    pKirby->Get_Body()->Set_KirbyEye(KIRBY_EYE_STATE::CLOSE);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(s_fMaxDamagedHorizontalSpeed);
}

void CKirby_Damaged::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    const _float fCurRatio = pAnimator->Get_Progress();

    const _float fStartRatio = 0.f;
    const _float fEndRatio = 0.25f;

    _float fRotRatio = fCurRatio / (fEndRatio - fStartRatio);
    Helper::FloatClamp(fRotRatio, 0.f, 1.f);

    _vector vAxis = XMVectorSet(1.f, 0.f, 0.f, 0.f);
    pAnimator->SetBoneRotation("RotL", fRotRatio * 360.f, vAxis);

    if (pAnimator->Get_Progress() >= fEndRatio)
    {
        pAnimator->SetBoneRotation("RotL", 0.f, vAxis);
        Transition_Fall_OR_Wait_OR_Run(pKirby);
    }
}

void CKirby_Damaged::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    pKirby->Get_Body()->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
}

_bool CKirby_Damaged::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

CKirby_Damaged* CKirby_Damaged::Create()
{
    CKirby_Damaged* pInstance = new CKirby_Damaged();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Damaged");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Damaged::Free()
{
    __super::Free();
}
