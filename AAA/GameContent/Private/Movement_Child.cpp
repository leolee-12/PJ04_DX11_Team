#include "Movement_Child.h"
#include "Transform.h"
#include "Controller.h"

#pragma warning(push, 0)
#ifdef new
#undef new
#endif
#include <PhysX/PxPhysicsAPI.h>
#if defined(_DEBUG) && defined(DBG_NEW)
#define new DBG_NEW
#endif
#pragma warning(pop)

NS_BEGIN(Client)

namespace
{
    constexpr _float EPSILON = 1e-6f;

    _float ClampFloat(_float v, _float minValue, _float maxValue)
    {
        if (v < minValue) return minValue;
        if (v > maxValue) return maxValue;
        return v;
    }

    // 현재에서 목표까지 직선 이동
    _vector MoveTowards(_fvector vCurrent, _fvector vTarget, _float fMaxDelta)
    {
        _vector vDelta = XMVectorSubtract(vTarget, vCurrent);
        _float fDist = XMVectorGetX(XMVector3Length(vDelta));

        if (fDist <= fMaxDelta || fDist <= EPSILON)
            return vTarget;

        return XMVectorAdd(vCurrent, XMVectorScale(XMVector3Normalize(vDelta), fMaxDelta));
    }
}

CMovement_Child::CMovement_Child(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMovement(pDevice, pContext)
    , m_fMass(1.f)

    , m_bUseGravity(true)
    , m_fRBGravity(-45.f)
    , m_fGravityScale(1.f)

    , m_fLinearDrag(0.9f)
    , m_bUseGroundFriction(true)
    , m_fGroundFriction(40.f)

    , m_fMaxHorizontalSpeed(8.f)
    , m_fMaxVerticalSpeed(30.f)
    , m_fMaxFallVelocity(-15.f)
    , m_fJumpVelocity(22.f)

    , m_bStopHorizontalOnSideHit(false)

    , m_fRotation_Speed_Degree(720.f)

    , m_fMaxCoyoteTime(0.1f)

    , m_fGroundPermitDistance(0.2f)
    , m_RayOriginOffsetFromFoot(0.15f)

    , m_vVelocity{ 0.f, 0.f, 0.f }
    , m_vForce{ 0.f, 0.f, 0.f }
    , m_vAcceleration{ 0.f, 0.f, 0.f }
{
}

CMovement_Child::CMovement_Child(const CMovement_Child& Prototype)
    : CMovement(Prototype)
    , m_fMass(Prototype.m_fMass)

    , m_bUseGravity(Prototype.m_bUseGravity)
    , m_fRBGravity(Prototype.m_fRBGravity)
    , m_fGravityScale(Prototype.m_fGravityScale)

    , m_fLinearDrag(Prototype.m_fLinearDrag)
    , m_bUseGroundFriction(Prototype.m_bUseGroundFriction)
    , m_fGroundFriction(Prototype.m_fGroundFriction)

    , m_fMaxHorizontalSpeed(Prototype.m_fMaxHorizontalSpeed)
    , m_fMaxVerticalSpeed(Prototype.m_fMaxVerticalSpeed)
    , m_fMaxFallVelocity(Prototype.m_fMaxFallVelocity)
    , m_fJumpVelocity(Prototype.m_fJumpVelocity)

    , m_bStopHorizontalOnSideHit(Prototype.m_bStopHorizontalOnSideHit)
    , m_fRotation_Speed_Degree(Prototype.m_fRotation_Speed_Degree)

    , m_fGroundPermitDistance(Prototype.m_fGroundPermitDistance)
    , m_RayOriginOffsetFromFoot(Prototype.m_RayOriginOffsetFromFoot)

    , m_fMaxCoyoteTime(Prototype.m_fMaxCoyoteTime)
    // Prototype의 런타임 물리 상태는 복사 x
    , m_vVelocity{ 0.f, 0.f, 0.f }
    , m_vForce{ 0.f, 0.f, 0.f }
    , m_vAcceleration{ 0.f, 0.f, 0.f }
{
}

HRESULT CMovement_Child::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMovement_Child::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    Stop();             // 속도 초기화
    Clear_Forces();     // 이번 프레임 힘, 가속도 초기화

    return S_OK;
}

_bool CMovement_Child::Update_RigidBody(_float fTimeDelta)
{
    if (m_pTransform == nullptr || m_pController == nullptr)
        return m_bGrounded;

    if (fTimeDelta <= 0.f)
        return m_bGrounded;

    // 프레임이 끝나도 속도 유지(로드)
    _vector vVelocity = XMLoadFloat3(&m_vVelocity);

    // 힘 -> 가속도(+ 중력) -> 속도로 변환
    Integrate_Forces(fTimeDelta, vVelocity);
    // 공기 저항, 바닥 마찰 적용
    Apply_Drag(fTimeDelta, vVelocity);
    // x, y 속도 제한, 낙하 속도 제한
    Clamp_Velocity(vVelocity);

    // PhysX Controller
    Move_Controller(vVelocity, fTimeDelta, vVelocity);
    Update_CoyoteTimer(fTimeDelta);

    // 수정된 Velocity 저장
    XMStoreFloat3(&m_vVelocity, vVelocity);
    // 부모 변수 동기화
    Sync_BaseVelocityFields();

    // 힘, 가속도 초기화
    Clear_Forces();

    return m_bGrounded;
}

_bool CMovement_Child::Check_GroundBelow()
{
    // PxController가 속한 Scene을 가져옴.
    physx::PxScene* pScene = m_pController->getScene();
    if (pScene == nullptr)
        return false;

    // 발 위치를 가지고 옴.
    const physx::PxExtendedVec3& vFoot = m_pController->getFootPosition();

    // Ray 시작 위치
    physx::PxVec3 vOrigin(
        static_cast<float>(vFoot.x),
        static_cast<float>(vFoot.y) + m_RayOriginOffsetFromFoot,
        static_cast<float>(vFoot.z)
    );

    // Ray 방향
    physx::PxVec3 vDir(0.f, -1.f, 0.f);

    // 총 거리
    const _float fMaxDistance = m_RayOriginOffsetFromFoot + m_fGroundPermitDistance;

    // 레이케스트 결과를 담는 버퍼
    physx::PxRaycastBuffer HitBuffer;

    // static들만 검사
    physx::PxQueryFilterData FilterData;
    FilterData.flags = physx::PxQueryFlag::eSTATIC;

    // ray 쏨
    const bool bHit = pScene->raycast(
        vOrigin,
        vDir,
        fMaxDistance,
        HitBuffer,
        physx::PxHitFlag::ePOSITION |
        physx::PxHitFlag::eNORMAL,
        FilterData
    );

    if (bHit == false || HitBuffer.hasBlock == false)
        return false;

    // 가장 가까운 충돌 결과를 꺼냄
    const physx::PxRaycastHit& Hit = HitBuffer.block;

    // 45도 이하만 바닥으로 인정
    const _float fMinGroundNormalY = cosf(physx::PxPi / 4.f);

    if (Hit.normal.y < fMinGroundNormalY)
        return false;

    return true;
}

void CMovement_Child::Add_Force(_fvector vValue, FORCE_MODE eMode)
{
    if (m_fMass <= EPSILON)
        m_fMass = 1.f;

    switch (eMode)
    {
    case FORCE_MODE::FORCE:
    {
        _vector vForce = XMVectorAdd(XMLoadFloat3(&m_vForce), vValue);
        XMStoreFloat3(&m_vForce, vForce);
        break;
    }
    case FORCE_MODE::ACCELERATION:
    {
        _vector vAccel = XMVectorAdd(XMLoadFloat3(&m_vAcceleration), vValue);
        XMStoreFloat3(&m_vAcceleration, vAccel);
        break;
    }
    case FORCE_MODE::IMPULSE:
    {
        _vector vDeltaVelocity = XMVectorScale(vValue, 1.f / m_fMass);
        Add_Velocity(vDeltaVelocity);
        break;
    }
    case FORCE_MODE::VELOCITY_CHANGE:
    {
        Add_Velocity(vValue);
        break;
    }
    default:
        break;
    }
}

void CMovement_Child::Add_Acceleration(_fvector vAccel)
{
    Add_Force(vAccel, FORCE_MODE::ACCELERATION);
}

void CMovement_Child::Add_Impulse(_fvector vImpulse)
{
    Add_Force(vImpulse, FORCE_MODE::IMPULSE);
}

void CMovement_Child::Add_VelocityChange(_fvector vDeltaVelocity)
{
    Add_Force(vDeltaVelocity, FORCE_MODE::VELOCITY_CHANGE);
}

_bool CMovement_Child::Try_Jump()
{
    return Try_Jump(m_fJumpVelocity);
}

_bool CMovement_Child::Try_Jump(_float fJumpVelocity)
{
    if (!m_bGrounded && m_fAccCoyoteTime <= 0.f)
        return false;

    Set_VelocityY(fJumpVelocity);
    m_bGrounded = false;
    m_fAccCoyoteTime = 0.f;
    return true;
}

void CMovement_Child::Force_Jump()
{
    Force_Jump(m_fJumpVelocity);
}

void CMovement_Child::Force_Jump(_float fJumpVelocity)
{
    Set_VelocityY(fJumpVelocity);
    m_bGrounded = false;
    m_fAccCoyoteTime = 0.f;
}

void CMovement_Child::Set_Velocity(_fvector vVelocity)
{
    XMStoreFloat3(&m_vVelocity, vVelocity);
    Sync_BaseVelocityFields();
}

void CMovement_Child::Set_VelocityX(_float fX)
{
    m_vVelocity.x = fX;
    Sync_BaseVelocityFields();
}

void CMovement_Child::Set_VelocityY(_float fY)
{
    m_vVelocity.y = fY;
    Sync_BaseVelocityFields();
}

void CMovement_Child::Set_VelocityZ(_float fZ)
{
    m_vVelocity.z = fZ;
    Sync_BaseVelocityFields();
}

void CMovement_Child::Add_Velocity(_fvector vDeltaVelocity)
{
    _vector vVelocity = XMVectorAdd(XMLoadFloat3(&m_vVelocity), vDeltaVelocity);
    XMStoreFloat3(&m_vVelocity, vVelocity);
    Sync_BaseVelocityFields();
}

_float CMovement_Child::Get_Speed() const
{
    return XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vVelocity)));
}

_float CMovement_Child::Get_HorizontalSpeed() const
{
    _vector vHoriz = XMVectorSet(m_vVelocity.x, 0.f, m_vVelocity.z, 0.f);
    return XMVectorGetX(XMVector3Length(vHoriz));
}

void CMovement_Child::Set_Mass(_float fMass)
{
    m_fMass = fMass <= EPSILON ? 1.f : fMass;
}

void CMovement_Child::Set_Gravity(_float fGravity)
{
    m_fRBGravity = fGravity;
}

void CMovement_Child::Set_GravityScale(_float fGravityScale)
{
    m_fGravityScale = fGravityScale;
}

void CMovement_Child::Set_LinearDrag(_float fLinearDrag)
{
    m_fLinearDrag = fLinearDrag < 0.f ? 0.f : fLinearDrag;
}

void CMovement_Child::Set_GroundFriction(_float fGroundFriction)
{
    m_fGroundFriction = fGroundFriction < 0.f ? 0.f : fGroundFriction;
}

void CMovement_Child::Set_MaxHorizontalSpeed(_float fMaxHorizontalSpeed)
{
    m_fMaxHorizontalSpeed = fMaxHorizontalSpeed < 0.f ? 0.f : fMaxHorizontalSpeed;
}

void CMovement_Child::Set_MaxVerticalSpeed(_float fMaxVerticalSpeed)
{
    m_fMaxVerticalSpeed = fMaxVerticalSpeed < 0.f ? 0.f : fMaxVerticalSpeed;
}

void CMovement_Child::Set_MaxFallVelocity(_float fMaxFallVelocity)
{
    // 아래 방향이 음수인 엔진 기준. 양수로 넣어도 음수로 보정한다.
    m_fMaxFallVelocity = fMaxFallVelocity > 0.f ? -fMaxFallVelocity : fMaxFallVelocity;
}

void CMovement_Child::Set_JumpVelocity(_float fJumpVelocity)
{
    m_fJumpVelocity = fJumpVelocity;
}

void CMovement_Child::Set_UseGravity(_bool bUseGravity)
{
    m_bUseGravity = bUseGravity;
}

void CMovement_Child::Set_UseGroundFriction(_bool bUseGroundFriction)
{
    m_bUseGroundFriction = bUseGroundFriction;
}

void CMovement_Child::Stop()
{
    m_vVelocity = { 0.f, 0.f, 0.f };
    Sync_BaseVelocityFields();
}

void CMovement_Child::Stop_Horizontal()
{
    m_vVelocity.x = 0.f;
    m_vVelocity.z = 0.f;
    Sync_BaseVelocityFields();
}

void CMovement_Child::Stop_Vertical()
{
    m_vVelocity.y = 0.f;
    Sync_BaseVelocityFields();
}

void CMovement_Child::Clear_Forces()
{
    m_vForce = { 0.f, 0.f, 0.f };
    m_vAcceleration = { 0.f, 0.f, 0.f };
}

void CMovement_Child::Sync_To_Controller()
{
    if (nullptr == m_pTransform || nullptr == m_pController)
        return;

    _float3 vPosition;
    XMStoreFloat3(&vPosition, m_pTransform->Get_State(STATE::POSITION));

    m_pController->setFootPosition(
        physx::PxExtendedVec3(vPosition.x, vPosition.y, vPosition.z)
    );

    Stop();
    Clear_Forces();

    m_bGrounded = false;
    Sync_BaseVelocityFields();
}

void CMovement_Child::Rotate_To_Direction(_fvector vDir, _float fTimeDelta)
{
    if (m_pTransform == nullptr)
        return;

    _vector vTargetDir = XMVectorSetY(vDir, 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vTargetDir)) <= EPSILON)
        return;

    vTargetDir = XMVector3Normalize(vTargetDir);

    _vector vLook = XMVectorSetY(m_pTransform->Get_State(STATE::LOOK), 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vLook)) <= EPSILON)
        return;

    vLook = XMVector3Normalize(vLook);

    _float fDot = XMVectorGetX(XMVector3Dot(vLook, vTargetDir));

    _float fCross =
        XMVectorGetZ(vLook) * XMVectorGetX(vTargetDir) -
        XMVectorGetX(vLook) * XMVectorGetZ(vTargetDir);

    _float fYaw = atan2f(fCross, fDot);

    _float fStep = XMConvertToRadians(m_fRotation_Speed_Degree) * fTimeDelta;

    _float fApply = fabsf(fYaw) <= fStep
        ? fYaw
        : (fYaw > 0.f ? fStep : -fStep);

    m_pTransform->Rotate(
        XMQuaternionRotationAxis(
            XMVectorSet(0.f, 1.f, 0.f, 0.f),
            fApply
        )
    );
}

void CMovement_Child::Integrate_Forces(_float fTimeDelta, _vector& vVelocity)
{
    if (m_fMass <= EPSILON)
        m_fMass = 1.f;

    // 힘 가속도 변환
    _vector vAccel = XMLoadFloat3(&m_vAcceleration);
    vAccel = XMVectorAdd(vAccel, XMVectorScale(XMLoadFloat3(&m_vForce), 1.f / m_fMass));

    // 중력 적용
    if (m_bUseGravity)
        vAccel = XMVectorAdd(vAccel, XMVectorSet(0.f, m_fRBGravity * m_fGravityScale, 0.f, 0.f));

    // 가속도에 시간을 곱해 속도 누적
    vVelocity = XMVectorAdd(vVelocity, XMVectorScale(vAccel, fTimeDelta));
}

void CMovement_Child::Apply_Drag(_float fTimeDelta, _vector& vVelocity)
{
    // 공기 저항
    if (m_fLinearDrag > 0.f)
    {
        // 1.f - m_fLinearDrag * fTimeDelta의 비율만큼 속도 감소
        _float fDragRatio = ClampFloat(1.f - m_fLinearDrag * fTimeDelta, 0.f, 1.f);
        vVelocity = XMVectorScale(vVelocity, fDragRatio);
    }

    // 바닥 마찰
    if (m_bGrounded && m_bUseGroundFriction && m_fGroundFriction > 0.f)
    {
        // x, z 속도만 추출
        _vector vHoriz = XMVectorSet(XMVectorGetX(vVelocity), 0.f, XMVectorGetZ(vVelocity), 0.f);
        // 수평 속도 -> 목표 속도(0)
        vHoriz = MoveTowards(vHoriz, XMVectorZero(), m_fGroundFriction * fTimeDelta);
        vVelocity = XMVectorSet(XMVectorGetX(vHoriz), XMVectorGetY(vVelocity), XMVectorGetZ(vHoriz), 0.f);
    }
}

void CMovement_Child::Clamp_Velocity(_vector& vVelocity)
{
    if (m_fMaxHorizontalSpeed > 0.f)
    {
        // x, z 속도 제한
        _vector vHoriz = XMVectorSet(XMVectorGetX(vVelocity), 0.f, XMVectorGetZ(vVelocity), 0.f);
        _float fSpeedSq = XMVectorGetX(XMVector3LengthSq(vHoriz));
        _float fMaxSq = m_fMaxHorizontalSpeed * m_fMaxHorizontalSpeed;

        if (fSpeedSq > fMaxSq && fSpeedSq > EPSILON)
        {
            vHoriz = XMVectorScale(XMVector3Normalize(vHoriz), m_fMaxHorizontalSpeed);
            vVelocity = XMVectorSet(XMVectorGetX(vHoriz), XMVectorGetY(vVelocity), XMVectorGetZ(vHoriz), 0.f);
        }
    }

    // 최대 수직 속도
    if (m_fMaxVerticalSpeed > 0.f && XMVectorGetY(vVelocity) > m_fMaxVerticalSpeed)
        vVelocity = XMVectorSetY(vVelocity, m_fMaxVerticalSpeed);

    // 최대 낙하 속도 제한
    if (XMVectorGetY(vVelocity) < m_fMaxFallVelocity)
        vVelocity = XMVectorSetY(vVelocity, m_fMaxFallVelocity);
}

void CMovement_Child::Move_Controller(_fvector vVelocity, _float fTimeDelta, _vector& vOutVelocity)
{
    // 변위 → CCT 이동 → Transform 위치 반영
    _float3 vDisp = {};
    XMStoreFloat3(&vDisp, XMVectorScale(vVelocity, fTimeDelta));

    physx::PxControllerFilters filters;
    filters.mFilterFlags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
    filters.mCCTFilterCallback = &Engine::Get_CCTFilter();

    physx::PxControllerCollisionFlags flags = m_pController->move(
        physx::PxVec3(vDisp.x, vDisp.y, vDisp.z),
        0.001f, fTimeDelta, filters);

    const physx::PxExtendedVec3& foot = m_pController->getFootPosition();
    m_pTransform->Set_State(STATE::POSITION,
        XMVectorSet(static_cast<_float>(foot.x), static_cast<_float>(foot.y), static_cast<_float>(foot.z), 1.f));

    // 접지 정리
    _bool bControllerGrounded = flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
    _bool bPermitGrounded = Check_GroundBelow();

    m_bGrounded = bControllerGrounded || bPermitGrounded;

    //// 천장에 머리 박았을 때 수직 속도 제거
    //if (flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_UP) && XMVectorGetY(vOutVelocity) > 0.f)
    //    vOutVelocity = XMVectorSetY(vOutVelocity, 0.f);

    // 벽에 비빌때 수평 속도 제거
    if (m_bStopHorizontalOnSideHit && flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_SIDES))
        vOutVelocity = XMVectorSet(0.f, XMVectorGetY(vOutVelocity), 0.f, 0.f);
}

void CMovement_Child::Sync_BaseVelocityFields()
{
    m_fVerticalVelocity = m_vVelocity.y;
    m_vHorizVel = { m_vVelocity.x, 0.f, m_vVelocity.z };
}

void CMovement_Child::Update_CoyoteTimer(_float fDeltaTime)
{
    if (m_bGrounded)
        m_fAccCoyoteTime = m_fMaxCoyoteTime;
    else
        m_fAccCoyoteTime -= fDeltaTime;

    if (m_fAccCoyoteTime < 0.f)
        m_fAccCoyoteTime = 0.f;
}

CMovement_Child* CMovement_Child::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMovement_Child* pInstance = new CMovement_Child(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMovement_Child");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CMovement_Child::Clone(void* pArg)
{
    CMovement_Child* pInstance = new CMovement_Child(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMovement_Child");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMovement_Child::Free()
{
    __super::Free();
}

NS_END