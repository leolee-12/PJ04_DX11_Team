#include "Movement.h"
#include "Transform.h"

#pragma warning(push, 0)
#ifdef new
#undef new
#endif
#include <PhysX/PxPhysicsAPI.h>
#if defined(_DEBUG) && defined(DBG_NEW)
#define new DBG_NEW
#endif
#pragma warning(pop)

CMovement::CMovement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext)
    , m_fMoveSpeed(6.f), m_fRotSpeedDeg(720.f)
    , m_fGravity(-20.f), m_fJumpSpeed(8.f), m_fMaxFallSpeed(-15.f)
    , m_fAccel(40.f), m_fDecel(50.f)
{
}

CMovement::CMovement(const CMovement& Prototype)
    : CComponent(Prototype)
    , m_fMoveSpeed(Prototype.m_fMoveSpeed), m_fRotSpeedDeg(Prototype.m_fRotSpeedDeg)
    , m_fGravity(Prototype.m_fGravity), m_fJumpSpeed(Prototype.m_fJumpSpeed)
    , m_fMaxFallSpeed(Prototype.m_fMaxFallSpeed)
    , m_fAccel(Prototype.m_fAccel), m_fDecel(Prototype.m_fDecel)
{
}

HRESULT CMovement::Initialize_Prototype() { return S_OK; }
HRESULT CMovement::Initialize(void* /*pArg*/) { return S_OK; }

void CMovement::Set_Refs(CTransform* pTransform, physx::PxController* pController)
{
    m_pController = pController;
    if (m_pTransform != pTransform)
    {
        Safe_AddRef(pTransform);
        Safe_Release(m_pTransform);
        m_pTransform = pTransform;
    }
}

void CMovement::Set_Stats(_float fMoveSpeed, _float fRotSpeedDeg, _float fGravity, _float fJumpSpeed)
{
    m_fMoveSpeed = fMoveSpeed;
    m_fRotSpeedDeg = fRotSpeedDeg;
    m_fGravity = fGravity;
    m_fJumpSpeed = fJumpSpeed;
}

void CMovement::Set_Acceleration(_float Accel, _float Decel)
{
    m_fAccel = Accel;
    m_fDecel = Decel;
}

void CMovement::Calc_Vertical(_float fTimeDelta)
{
    m_fVerticalVelocity += m_fGravity * fTimeDelta;
    if (m_fVerticalVelocity < m_fMaxFallSpeed)
        m_fVerticalVelocity = m_fMaxFallSpeed;
}

_bool CMovement::Move(_fvector vWishDir, _float fTimeDelta)
{
    if (nullptr == m_pTransform || nullptr == m_pController)
        return m_bGrounded;

    // 1) 목표 수평속도 + 이동방향 회전
    _bool   bHasInput = !XMVector3Equal(vWishDir, XMVectorZero());
    _vector vTargetVel = XMVectorZero();
    if (bHasInput)
    {
        _vector vDir = XMVector3Normalize(XMVectorSetY(vWishDir, 0.f));
        vTargetVel = vDir * m_fMoveSpeed;

        // 입력 방향으로 부드럽게 회전(현재 LOOK 기준)
        _vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransform->Get_State(STATE::LOOK), 0.f));
        _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vDir));
        _float  fCross = XMVectorGetZ(vLook) * XMVectorGetX(vDir)
            - XMVectorGetX(vLook) * XMVectorGetZ(vDir);
        _float  fYaw = atan2f(fCross, fDot);
        _float  fStep = XMConvertToRadians(m_fRotSpeedDeg) * fTimeDelta;
        _float  fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
        m_pTransform->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
    }

    // 현재 속도를 목표로 가속/감속 (MoveTowards)
    _float  fRate = (bHasInput ? m_fAccel : m_fDecel) * fTimeDelta;
    _vector vCur = XMLoadFloat3(&m_vHorizVel);
    _vector vDelta = XMVectorSubtract(vTargetVel, vCur);
    _float  fDist = XMVectorGetX(XMVector3Length(vDelta));
    if (fDist <= fRate || fDist < 1e-5f)
        vCur = vTargetVel;                                   // 목표 도달
    else
        vCur += XMVector3Normalize(vDelta) * fRate;          // 목표 쪽으로 한 스텝
    XMStoreFloat3(&m_vHorizVel, vCur);

    _float3 vHoriz;
    XMStoreFloat3(&vHoriz, vCur * fTimeDelta);

    // 2) 점프 의도 소비 + 중력
    if (m_bGrounded && m_bJumpQueued)
        m_fVerticalVelocity = m_fJumpSpeed;
    m_bJumpQueued = false;     // 매 프레임 소비(땅에 없으면 그냥 버려짐)

    Calc_Vertical(fTimeDelta); // 중력/낙하 (virtual 훅)

    // 3) 변위 → CCT 이동 → Transform 위치 반영
    physx::PxControllerFilters filters;
    filters.mFilterFlags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

    physx::PxControllerCollisionFlags flags = m_pController->move(
        physx::PxVec3(vHoriz.x, m_fVerticalVelocity * fTimeDelta, vHoriz.z),
        0.001f, fTimeDelta, filters);

    const physx::PxExtendedVec3& foot = m_pController->getFootPosition();
    m_pTransform->Set_State(STATE::POSITION,
        XMVectorSet((_float)foot.x, (_float)foot.y, (_float)foot.z, 1.f));

    // 4) 접지 정리
    m_bGrounded = flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
    if (m_bGrounded && m_fVerticalVelocity < 0.f)
        m_fVerticalVelocity = 0.f;

    return m_bGrounded;
}

void CMovement::Jump()
{
    m_bJumpQueued = true;
}

void CMovement::Sync_To_Controller()
{
    if (nullptr == m_pTransform || nullptr == m_pController)
        return;

    _float3 p; XMStoreFloat3(&p, m_pTransform->Get_State(STATE::POSITION));
    m_pController->setFootPosition(physx::PxExtendedVec3(p.x, p.y, p.z));

    m_fVerticalVelocity = 0.f;
    m_bGrounded = false;
    m_vHorizVel = {};
    m_bJumpQueued = false;
}

CMovement* CMovement::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMovement* pInstance = new CMovement(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMovement");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CMovement::Clone(void* pArg)
{
    CMovement* pInstance = new CMovement(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMovement");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMovement::Free()
{
    Safe_Release(m_pTransform);   // Set_Refs에서 AddRef한 것 해제 (CCT는 캐릭터가 소유)
    __super::Free();
}