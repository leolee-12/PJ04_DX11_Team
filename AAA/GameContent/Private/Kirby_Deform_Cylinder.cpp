#include "Kirby_Deform_Cylinder.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_State.h"

#include "LD_DeformObject.h"
#include "Kirby_Deform.h"

#include "Movement_Child.h"

#include "Deformable.h"

#include "Kirby_DeformDump.h"

CKirby_Deform_Cylinder::CKirby_Deform_Cylinder()
{
}

HRESULT CKirby_Deform_Cylinder::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"토관 머금기";

    Set_FullBodyAni(DEFORM_ANI::SPIT_START, "SpitStart", false, false, 0.1f, 1.5f);

    return S_OK;
}

DEFORM_TYPE CKirby_Deform_Cylinder::Get_DeformType()
{
    return DEFORM_TYPE::CYLINDER;
}

void CKirby_Deform_Cylinder::Enter_Deform(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(s_fCylinderMaxHorizontalSpeed);
    pMovement->Set_GravityScale(2.f);

    // CCT
    pKirby->Set_CCTSize(s_fCylinder_CCT_Radius, s_fCylinder_CCT_Height);

    constexpr _float fRadiusPadding = 0.5f;

    // Hurt Box
    CCollider::COLLIDER_DESC tHurtDesc{};
    tHurtDesc.pOwner = pKirby;
    tHurtDesc.fRadius = 1.8f;
    tHurtDesc.fHeight = 4.7f;
    tHurtDesc.vCenter =
    {
        tHurtDesc.fHeight / 2.f + tHurtDesc.fRadius,
        tHurtDesc.fRadius / 2.f + fRadiusPadding,
        0.f
    };
    tHurtDesc.vRadians = _float3(0.f, 0.f, XMConvertToRadians(90.f));
    pKirby->Set_ColliderDesc(CKirby::HURT_BOX, tHurtDesc);

    // Hit Box
    CCollider::COLLIDER_DESC tBreakerableHitDesc{};
    tBreakerableHitDesc.pOwner = pKirby;
    tBreakerableHitDesc.fRadius = 2.3f;
    tBreakerableHitDesc.fHeight = 4.7f;
    tBreakerableHitDesc.vCenter =
    {
        tBreakerableHitDesc.fHeight / 2.f + tBreakerableHitDesc.fRadius,
        tBreakerableHitDesc.fRadius / 2.f + fRadiusPadding,
        0.f
    };
    tBreakerableHitDesc.vRadians = _float3(0.f, 0.f, XMConvertToRadians(90.f));
    pKirby->Set_ColliderDesc(CKirby::BREAKERABLE_HITBOX, tBreakerableHitDesc);
}

void CKirby_Deform_Cylinder::Exit_Deform(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
    pMovement->Set_GravityScale(1.f);

    pKirby->Set_CCTSize(CKirby::s_fCCT_Radius, CKirby::s_fCCT_Height);

    // Hurt Box
    CCollider::COLLIDER_DESC tHurtDesc{};
    tHurtDesc.pOwner = pKirby;
    tHurtDesc.vCenter = _float3(0.f, 0.f, 0.f);
    tHurtDesc.fRadius = CKirby::s_fCCT_Radius + CKirby::s_fHurtBoxRadiusPadding;
    tHurtDesc.fHeight = CKirby::s_fCCT_Height;
    pKirby->Set_ColliderDesc(CKirby::HURT_BOX, tHurtDesc);
}

void CKirby_Deform_Cylinder::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bReqEndAttackState = false;
    m_bCurMoveInput = false;
    m_bPreMoveInput = false;

    m_eCylinderState = DEFORM_CYLINDER_STATE::CYLINDER_STATE_END;
    Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::ROT_MOVEDIR);
}

void CKirby_Deform_Cylinder::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_DeformCylinderState(pKirby, fTimeDelta);
}

void CKirby_Deform_Cylinder::Exit_AttackState(CKirby* pKirby)
{
    m_bCurMoveInput = false;
    m_bPreMoveInput = false;
}

_bool CKirby_Deform_Cylinder::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();
    //m_vRotDir
    switch (eCommandType)
    {        
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_eCylinderState == DEFORM_CYLINDER_STATE::CLASHED_WAIT)
                m_bCurMoveInput = true;

            return true;
        }

        // Dump
        case KIRBY_COMMAND_TYPE::DUMP:
        {
            if (!pCommand->IsPress())
                return false;

            if(m_eCylinderState == DEFORM_CYLINDER_STATE::CLASHED_WAIT)
            {
                if (pKirby->Can_Dump() == true)
                {
                    Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CYLINDER_STATE_END);
                    pKirby->Reset_DumpCool();
                    return true;
                }

                pKirby->Req_AbilityDumpCoolDecrease();
            }

            return true;
        }
        // Jump Down
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            m_bTryJump = true;

            return true;
        }
    }

    return false;
}

void CKirby_Deform_Cylinder::Enter_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    _float fMaxHorizontalSpeed = 35.f;
    pMovement->Set_MaxHorizontalSpeed(fMaxHorizontalSpeed);
    pMovement->Set_MaxFallVelocity(s_fCylinderCarFallVelocity);

    m_fMoveDir = DeformContext.vStartLook;
}

_bool CKirby_Deform_Cylinder::Update_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext, _float fTimeDelta)
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

void CKirby_Deform_Cylinder::Exit_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
    pMovement->Set_MaxFallVelocity(CKirby::s_fMaxFallVelocity);
}

void CKirby_Deform_Cylinder::Change_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eNext)
{
    if (m_eCylinderState == eNext)
        return;

    Exit_DeformCylinderState(pKirby, m_eCylinderState);

    m_eCylinderState = eNext;

    Enter_DeformCylinderState(pKirby, m_eCylinderState);
}

void CKirby_Deform_Cylinder::Enter_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eState)
{
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CYLINDER);
    CAnimator* pAnimator = pModel->Get_Animator();

    switch (eState)
    {
        case DEFORM_CYLINDER_STATE::ROT_MOVEDIR:
        {
            break;
        }
        case DEFORM_CYLINDER_STATE::ROLL:
        {
            m_RollSound = m_pGameInstance_Proxy->Play_SFX_Section_Loop(L"HeroDeformCylinder_RollingFast.wav", 0.0829f, 0.2959f, 0.2f);

            m_bTryJump = false;
            m_eRollState = ROLL_STATE::ROLL_STATE_END;
            Change_RollState(pKirby, ROLL_STATE::MOVE);
            pKirby->Set_ColliderEnabled(CKirby::BREAKERABLE_HITBOX, true);
            break;
        }

        case DEFORM_CYLINDER_STATE::CLASH:
        {
            pAnimator->Play("Clash", false, false, 0.1f, 1.5f);
            m_fRotL_Degree = 0.f;
            pAnimator->SetBoneRotation("RotL", m_fRotL_Degree, XMVectorSet(1.f, 0.f, 0.f, 0.f));

            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Stop();
            pMovement->Set_GravityScale(0.f);
            break;
        }
        case DEFORM_CYLINDER_STATE::CLASH_SLIP_WALL:
        {
            pAnimator->Play("ClashedSlipWall", true, false, 0.1f, 1.5f);
            break;
        }
        case DEFORM_CYLINDER_STATE::CLASHED_REMOVE_WALL:
        {
            pAnimator->Play("ClashedRemoveWall", false, false, 0.1f, 1.5f);
            break;
        }

        case DEFORM_CYLINDER_STATE::CLASHED_LANDING:
        {
            pAnimator->Play("ClashedLanding", false, false, 0.1f, 1.5f);
            break;
        }

        case DEFORM_CYLINDER_STATE::CLASHED_WAIT:
        {
            pAnimator->Play("ClashedWait", true, false, 0.1f, 1.5f);
            break;
        }
        case DEFORM_CYLINDER_STATE::CYLINDER_STATE_END:
        {
            m_bReqEndAttackState = true;
            pKirby->Change_State(KIRBY_STATE_TYPE::DEFORM_DUMP, DEFORM_DUMP_STATE_FLAG::SPIT_DEFORM_JUMP);
            break;
        }
    }
}

void CKirby_Deform_Cylinder::Update_DeformCylinderState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CYLINDER);
    CAnimator* pCylinderAnimator = pModel->Get_Animator();

    switch (m_eCylinderState)
    {
        case DEFORM_CYLINDER_STATE::ROT_MOVEDIR:
        {
            Rot_MoveDir(pKirby, fTimeDelta);
            break;
        }
        case DEFORM_CYLINDER_STATE::ROLL:
        {
            if (m_pGameInstance_Proxy->Key_Down(DIK_F))
            {
                Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CLASH);
                return;
            }
            if (pKirby->Get_DeformEndTrigger() && Check_FrontCollision(pKirby))
            {
                Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CLASH);
                return;
            }

            Roll(pKirby, fTimeDelta);
            Roll_RotL(pKirby, fTimeDelta);
            Update_RollState(pKirby, fTimeDelta);
            break;
        }
        case DEFORM_CYLINDER_STATE::CLASH:
        {
            if (pCylinderAnimator->Is_Finished())
            {
                CMovement_Child* pMovement = pKirby->Get_Movement();

                if (pMovement->Is_Grounded())
                    Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CLASHED_REMOVE_WALL);
                else
                    Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CLASH_SLIP_WALL);
            }
            break;
        }

        case DEFORM_CYLINDER_STATE::CLASH_SLIP_WALL:
        {           
            if (pKirby->Get_Movement()->Is_Grounded())
                Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CLASHED_REMOVE_WALL);
            break;
        }
        case DEFORM_CYLINDER_STATE::CLASHED_REMOVE_WALL:
        {
            if (pCylinderAnimator->Is_Finished())
                Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CLASHED_LANDING);
            break;
        }

        case DEFORM_CYLINDER_STATE::CLASHED_LANDING:
        {
            if (pCylinderAnimator->Is_Finished())
                Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::CLASHED_WAIT);
            break;
        }
        case DEFORM_CYLINDER_STATE::CLASHED_WAIT:
        {
            if (m_bCurMoveInput != m_bPreMoveInput)
            {
                if (m_bCurMoveInput == true)
                    pCylinderAnimator->Play("ClashedMove", true, false, 0.1f, 2.f);
                else
                    pCylinderAnimator->Play("ClashedWait", true, false, 0.1f, 1.5f);

                m_bPreMoveInput = m_bCurMoveInput;
            }

            m_bCurMoveInput = false;
            break;
        }
    }
}

void CKirby_Deform_Cylinder::Exit_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eState)
{
    switch (eState)
    {
        case DEFORM_CYLINDER_STATE::ROT_MOVEDIR:
        {
            break;
        }
        case DEFORM_CYLINDER_STATE::ROLL:
        {
            Change_RollState(pKirby, ROLL_STATE::ROLL_STATE_END);
            pKirby->Set_ColliderEnabled(CKirby::BREAKERABLE_HITBOX, false);
            break;
        }
        case DEFORM_CYLINDER_STATE::CYLINDER_STATE_END:
        {
            break;
        }
        case DEFORM_CYLINDER_STATE::CLASH:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_GravityScale(1.f);
            break;
        }
        case DEFORM_CYLINDER_STATE::CLASH_SLIP_WALL:
            break;
        case DEFORM_CYLINDER_STATE::CLASHED_REMOVE_WALL:
            break;
        case DEFORM_CYLINDER_STATE::CLASHED_LANDING:
            break;
        case DEFORM_CYLINDER_STATE::CLASHED_WAIT:
            break;
    }
}

void CKirby_Deform_Cylinder::Rot_MoveDir(CKirby* pKirby, _float fTimeDelta)
{
    _vector vTargetDir = XMLoadFloat3(&m_fMoveDir);
    vTargetDir = XMVectorSetY(vTargetDir, 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vTargetDir)) <= Helper::fEpsilon)
    {
        assert(false);
        return;
    }

    vTargetDir = XMVector3Normalize(vTargetDir);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Rotate_To_Direction(vTargetDir, fTimeDelta);

    CTransform* pTransform = pKirby->Get_Transform();
    _vector vCurrentLook =XMVector3Normalize(XMVectorSetY(pTransform->Get_State(STATE::LOOK), 0.f));

    constexpr _float fFinishDegree = 1.f;
    const _float fFinishDot = cosf(XMConvertToRadians(fFinishDegree));

    if (XMVectorGetX(XMVector3Dot(vCurrentLook, vTargetDir)) >= fFinishDot)
    {
        pTransform->LookTo(vTargetDir);
        Change_DeformCylinderState(pKirby, DEFORM_CYLINDER_STATE::ROLL);
    }
}

void CKirby_Deform_Cylinder::Roll(CKirby* pKirby, _float fTimeDelta)
{
    _vector vLook = pKirby->Get_Transform()->Get_State(STATE::LOOK);
    vLook = XMVectorSetY(vLook, 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vLook)) <= Helper::fEpsilon)
    {
        assert(false);
        return;
    }

    vLook = XMVector3Normalize(vLook);

    constexpr _float fRollAcceleration = 70.f;
    pKirby->Get_Movement()->Add_Acceleration(vLook * fRollAcceleration);
}

void CKirby_Deform_Cylinder::Roll_RotL(CKirby* pKirby, _float fTimeDelta)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    _float3 vVelocity = pMovement->Get_Velocity();
    _vector vSpeedXZ = XMLoadFloat3(&vVelocity);
    vSpeedXZ = XMVectorSetY(vSpeedXZ, 0.f);

    _float fSpeedXZ = XMVectorGetX(XMVector3Length(vSpeedXZ));
    if (fSpeedXZ < Helper::fEpsilon)
        return;

    constexpr _float fRotScale = 40.f;
    m_fRotL_Degree += fSpeedXZ * fRotScale * fTimeDelta;
    m_fRotL_Degree = fmodf(m_fRotL_Degree, 360.f);

    CAnimator* pAnimator = pKirby->Get_CurrentDeformModel()->Get_Animator();
    pAnimator->SetBoneRotation("RotL", m_fRotL_Degree, XMVectorSet(-1.f, 0.f, 0.f, 0.f));
}

_bool CKirby_Deform_Cylinder::Check_FrontCollision(CKirby* pKirby)
{
    CTransform* pTransform = pKirby->Get_Transform();

    _float3 vCenter{};
    XMStoreFloat3(&vCenter, pTransform->Get_State(STATE::POSITION));
    vCenter.y += s_fCylinder_CCT_Radius;

    _float3 vDir{};
    XMStoreFloat3(&vDir, XMVector3Normalize(pTransform->Get_State(STATE::LOOK)));

    _float fRadius = 0.1f;
    _float3 vDumpNormal{};
    _float  fDumpDistance{};
    _float fCollisionRadiusPadding = 0.2f;
    return m_pGameInstance_Proxy->Sweep_Sphere(
        vCenter, fRadius, vDir,
        s_fCylinder_CCT_Radius + fCollisionRadiusPadding,
        &vDumpNormal, &fDumpDistance, true, false);
}

void CKirby_Deform_Cylinder::Change_RollState(CKirby* pKirby, ROLL_STATE eNext)
{
    if (m_eRollState == eNext)
        return;

    Exit_RollState(pKirby, m_eRollState);

    m_eRollState = eNext;

    Enter_RollState(pKirby, m_eRollState);
}

void CKirby_Deform_Cylinder::Enter_RollState(CKirby* pKirby, ROLL_STATE eState)
{
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CYLINDER);

    CAnimator* pAnimator = pModel->Get_Animator();

    switch (eState)
    {
        case ROLL_STATE::MOVE:
            if (m_RollSound.Is_Valid())
                m_RollSound.Set_Paused(false);
            pAnimator->Play("Rolling", true, false, 0.1f, 1.5f);
            break;

        case ROLL_STATE::JUMP:
            m_pGameInstance_Proxy->Play_SFX(L"HeroDeformCylinder_Jump.wav", 0.2f);
            pAnimator->Play("Rolling", true, false, 0.1f, 1.5f);            
            break;

        case ROLL_STATE::FALL:
            pAnimator->Play("Fall", true, false, 0.1f, 1.5f);
            break;

        case ROLL_STATE::LANDING:
            if (m_RollSound.Is_Valid())
                m_RollSound.Set_Paused(false);
            pAnimator->Play("Landing", false, false, 0.1f, 2.f);
            break;

        case ROLL_STATE::ROLL_STATE_END:
            if (m_RollSound.Is_Valid())
                m_RollSound.Stop();
            break;
    }
}

void CKirby_Deform_Cylinder::Update_RollState(CKirby* pKirby, _float fTimeDelta)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    constexpr _float fJumpSpeed = 27.f;

    switch (m_eRollState)
    {
        case ROLL_STATE::MOVE:
        {
            if(m_bTryJump && pMovement->Try_Jump(fJumpSpeed))
                Change_RollState(pKirby, ROLL_STATE::JUMP);
            else if(!pMovement->Is_Grounded())
                Change_RollState(pKirby, ROLL_STATE::FALL);

            break;
        }

        case ROLL_STATE::JUMP:
        {
            if (pMovement->Get_VerticalVelocity() <= 0.f)
                Change_RollState(pKirby, ROLL_STATE::FALL);
            break;
        }

        case ROLL_STATE::FALL:
        {
            if (m_bTryJump && pMovement->Try_Jump(fJumpSpeed))
                Change_RollState(pKirby, ROLL_STATE::JUMP);
            else if (pMovement->Is_Grounded())
                Change_RollState(pKirby, ROLL_STATE::LANDING);
            break;
        }
        case ROLL_STATE::LANDING:
        {
            CAnimator* pAnimator = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CYLINDER)->Get_Animator();
            _bool bIsGround = pMovement->Is_Grounded();

            if (m_bTryJump && pMovement->Try_Jump(fJumpSpeed))
                Change_RollState(pKirby, ROLL_STATE::JUMP);
            else if (!bIsGround)
                Change_RollState(pKirby, ROLL_STATE::FALL);
            else if (pAnimator->Is_Finished())
                Change_RollState(pKirby, ROLL_STATE::MOVE);

            break;
        }
    }

    m_bTryJump = false;
}

void CKirby_Deform_Cylinder::Exit_RollState(CKirby* pKirby, ROLL_STATE eState)
{
    switch (eState)
    {
        case ROLL_STATE::MOVE:
        {
            if (m_RollSound.Is_Valid())
                m_RollSound.Set_Paused(true);
            break;
        }

        case ROLL_STATE::JUMP:
            break;

        case ROLL_STATE::FALL:
            break;

        case ROLL_STATE::LANDING:
            break;
    }
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
