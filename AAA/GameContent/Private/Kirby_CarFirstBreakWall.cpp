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

    m_bTurnStarted = false;

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
    
    CMovement_Child* pMovement = pKirby->Get_Movement();

    _float fRatio = pAnimator->Get_Progress();

    CTransform* pTransform = pKirby->Get_Transform();

    //// È¸Àü
    //if (fRatio >= 0.25f && m_bTurnStarted == false)
    //{
    //    m_bTurnStarted = true;
    //    _vector vLeft = -pTransform->Get_State(STATE::RIGHT);

    //    XMStoreFloat3(&m_vLeftDir, XMVector3Normalize(XMVectorSetY(vLeft, 0.f)));
    //}
    //if (m_bTurnStarted && fRatio <= 0.75f)
    //{
    //    pMovement->Rotate_To_Direction(XMLoadFloat3(&m_vLeftDir), fTimeDelta);
    //}

    /*if(fRatio <= 0.75f)
    {
        _vector vLook = pTransform->Get_State(STATE::LOOK);
        vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));
        pMovement->Add_Acceleration(vLook * 70.f);
    }*/

    pKirby->Update_CutsceneGrabTransform();

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

void CKirby_CarFirstBreakWall::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
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
