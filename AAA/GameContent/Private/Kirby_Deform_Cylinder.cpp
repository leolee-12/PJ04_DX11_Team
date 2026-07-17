#include "Kirby_Deform_Cylinder.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

#include "Movement_Child.h"

#include "Deformable.h"

CKirby_Deform_Cylinder::CKirby_Deform_Cylinder()
{
}

HRESULT CKirby_Deform_Cylinder::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

DEFORM_TYPE CKirby_Deform_Cylinder::Get_DeformType()
{
    return DEFORM_TYPE::CYLINDER;
}

void CKirby_Deform_Cylinder::Enter_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_Cylinder::Exit_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_Cylinder::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
}

void CKirby_Deform_Cylinder::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Deform_Cylinder::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Deform_Cylinder::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
}

_bool CKirby_Deform_Cylinder::Enter_Attack_KeyDown(CKirby* pKirby)
{
    return true;
}

_bool CKirby_Deform_Cylinder::Enter_Attack_KeyPress(CKirby* pKirby)
{
    return true;
}

_bool CKirby_Deform_Cylinder::Enter_Attack_KeyUp(CKirby* pKirby)
{
    return true;
}

void CKirby_Deform_Cylinder::Enter_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    _float fMaxHorizontalSpeed = 30.f;
    pMovement->Set_MaxHorizontalSpeed(fMaxHorizontalSpeed);
}

_bool CKirby_Deform_Cylinder::Update_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext, _float fTimeDelta)
{
    CKirby_Deform_Model* pDeformModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CYLINDER, KIRBY_DEFORM_MODEL_TYPE::DEMO);
    _float fRatio = pDeformModel->Get_Animator()->Get_Progress();

    if (fRatio < 0.55f)
        return false;

    _vector vTargetPos = XMLoadFloat3(&DeformContext.vStartPos);

    CTransform* pTransform = pKirby->Get_Transform();
    _vector vCurPos = pTransform->Get_State(STATE::POSITION);
    _vector vMoveDir = XMVectorSetY(vTargetPos - vCurPos, 0.f);

    constexpr _float fSpeed = 40.f;
    _float fEstimatedFrameMoveDist = fSpeed * fTimeDelta;
    const _float fArrivalDistanceThreshold = fEstimatedFrameMoveDist > 0.1f ? fEstimatedFrameMoveDist : 0.1f;

    CMovement_Child* pMovement = pKirby->Get_Movement();
    if (XMVectorGetX(XMVector3LengthSq(vMoveDir)) < fArrivalDistanceThreshold * fArrivalDistanceThreshold)
    {
        pTransform->Set_State(STATE::POSITION, vTargetPos);
        pMovement->Sync_To_Controller();
        return true;
    }

    vMoveDir = XMVector3Normalize(vMoveDir);

    pMovement->Set_Velocity(vMoveDir * fSpeed);

    return false;
}

void CKirby_Deform_Cylinder::Exit_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
}

CKirby_Deform_Cylinder* CKirby_Deform_Cylinder::Create()
{
    CKirby_Deform_Cylinder* pInstance = new CKirby_Deform_Cylinder();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Deform_Cylinder");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Deform_Cylinder::Free()
{
    __super::Free();
}
