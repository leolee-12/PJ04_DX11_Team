#include "Kirby_CarFirstBreakWall.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_CarFirstBreakWall::CKirby_CarFirstBreakWall()
{
}

HRESULT CKirby_CarFirstBreakWall::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_CarFirstBreakWall::Get_StateType()
{
    return KIRBY_STATE_TYPE::CAR_FIRST_BREAK_WALL;
}

void CKirby_CarFirstBreakWall::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR, KIRBY_DEFORM_MODEL_TYPE::MAIN);
    CAnimator* pAnimator = pModel->Get_Animator();
    pAnimator->Play("DeformCarGetFirst", false, false, 0.1f, 1.5f);
}

void CKirby_CarFirstBreakWall::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR, KIRBY_DEFORM_MODEL_TYPE::MAIN);
    CAnimator* pAnimator = pModel->Get_Animator();
    
    if(pAnimator->Is_Finished())
        Transition_Fall_OR_Wait_OR_Run(pKirby);
}

void CKirby_CarFirstBreakWall::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_CarFirstBreakWall::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_CarFirstBreakWall::Request_ReleaseGrabState(CKirby* pKirby, CUTSCENE_KIRBY_TYPE eType)
{
    Transition_Fall_OR_Wait_OR_Run(pKirby);
}

CKirby_CarFirstBreakWall* CKirby_CarFirstBreakWall::Create()
{
    CKirby_CarFirstBreakWall* pInstance = new CKirby_CarFirstBreakWall();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_CarFirstBreakWall");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_CarFirstBreakWall::Free()
{
    __super::Free();
}
