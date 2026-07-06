#include "Bounding_Capsule.h"
#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "DebugDraw.h"
#include "DebugCapsule.h"

// ---- 수학 헬퍼 ----
static _float Clamp01(_float t) { return t < 0.f ? 0.f : (t > 1.f ? 1.f : t); }

static _float Sq_Dist_Point_Segment(XMVECTOR P, XMVECTOR A, XMVECTOR B)
{
    XMVECTOR AB = B - A, AP = P - A;
    _float denom = XMVectorGetX(XMVector3Dot(AB, AB));
    _float t = (denom > 1e-8f) ? XMVectorGetX(XMVector3Dot(AP, AB)) / denom : 0.f;
    t = Clamp01(t);
    XMVECTOR d = P - (A + AB * t);
    return XMVectorGetX(XMVector3Dot(d, d));
}

// 두 선분 최근접 거리² (Ericson, RTCD)
static _float Sq_Dist_Segment_Segment(XMVECTOR P1, XMVECTOR Q1, XMVECTOR P2, XMVECTOR Q2)
{
    XMVECTOR d1 = Q1 - P1, d2 = Q2 - P2, r = P1 - P2;
    _float a = XMVectorGetX(XMVector3Dot(d1, d1));
    _float e = XMVectorGetX(XMVector3Dot(d2, d2));
    _float f = XMVectorGetX(XMVector3Dot(d2, r));
    const _float EPS = 1e-8f;
    _float s, t;

    if (a <= EPS && e <= EPS) { XMVECTOR dv = P1 - P2; return XMVectorGetX(XMVector3Dot(dv, dv)); }
    if (a <= EPS) { s = 0.f; t = Clamp01(f / e); }
    else
    {
        _float c = XMVectorGetX(XMVector3Dot(d1, r));
        if (e <= EPS) { t = 0.f; s = Clamp01(-c / a); }
        else
        {
            _float b = XMVectorGetX(XMVector3Dot(d1, d2));
            _float denom = a * e - b * b;
            s = (denom > EPS) ? Clamp01((b * f - c * e) / denom) : 0.f;
            t = (b * s + f) / e;
            if (t < 0.f) { t = 0.f; s = Clamp01(-c / a); }
            else if (t > 1.f) { t = 1.f; s = Clamp01((b - c) / a); }
        }
    }
    XMVECTOR dv = (P1 + d1 * s) - (P2 + d2 * t);
    return XMVectorGetX(XMVector3Dot(dv, dv));
}

// ---- 본체 ----
CBounding_Capsule::CBounding_Capsule(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBounding{ pDevice, pContext } {
}

HRESULT CBounding_Capsule::Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc)
{
    auto pDesc = static_cast<const BOUNDING_CAPSULE_DESC*>(pBoundingDesc);
    m_fRadius = pDesc->fRadius;
    _float fCyl = pDesc->fHeight;                 // 원기둥 길이(반구중심 간)

    XMVECTOR q = XMQuaternionRotationRollPitchYaw(pDesc->vRadians.x, pDesc->vRadians.y, pDesc->vRadians.z);
    XMVECTOR axis = XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), q);   // 위 방향

    XMVECTOR foot = XMLoadFloat3(&pDesc->vCenter);   // ← vCenter = 발(맨 아래)

    XMVECTOR pBottom = foot + axis * m_fRadius;              // 아래 반구중심
    XMVECTOR pTop = foot + axis * (m_fRadius + fCyl);     // 위  반구중심

    XMStoreFloat3(&m_vLocalP0, pTop);
    XMStoreFloat3(&m_vLocalP1, pBottom);

    // 초기 월드값 = 로컬값
    m_vP0 = m_vLocalP0; m_vP1 = m_vLocalP1; m_fWorldRadius = m_fRadius;
    return S_OK;
}

void CBounding_Capsule::Update(_fmatrix M)
{
    XMStoreFloat3(&m_vP0, XMVector3Transform(XMLoadFloat3(&m_vLocalP0), M));
    XMStoreFloat3(&m_vP1, XMVector3Transform(XMLoadFloat3(&m_vLocalP1), M));

    _float sx = XMVectorGetX(XMVector3Length(M.r[0]));
    _float sz = XMVectorGetX(XMVector3Length(M.r[2]));
    m_fWorldRadius = m_fRadius * max(sx, sz);     // 측면 스케일 반영
}

void CBounding_Capsule::Reset_Desc(const CBounding::BOUNDING_DESC* pDesc)
{
    auto pCapDesc = static_cast<const BOUNDING_CAPSULE_DESC*>(pDesc);

    m_fRadius = pCapDesc->fRadius;
    _float fCyl = pCapDesc->fHeight;

    XMVECTOR q = XMQuaternionRotationRollPitchYaw(pCapDesc->vRadians.x, pCapDesc->vRadians.y, pCapDesc->vRadians.z);
    XMVECTOR axis = XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), q);
    XMVECTOR foot = XMLoadFloat3(&pCapDesc->vCenter);

    XMVECTOR pBottom = foot + axis * m_fRadius;
    XMVECTOR pTop = foot + axis * (m_fRadius + fCyl);

    XMStoreFloat3(&m_vLocalP0, pTop);
    XMStoreFloat3(&m_vLocalP1, pBottom);

    m_vP0 = m_vLocalP0; m_vP1 = m_vLocalP1; m_fWorldRadius = m_fRadius;
}

_bool CBounding_Capsule::Intersects_Sphere(const BoundingSphere* pSphere) const
{
    _float dSq = Sq_Dist_Point_Segment(XMLoadFloat3(&pSphere->Center),
        XMLoadFloat3(&m_vP0), XMLoadFloat3(&m_vP1));
    _float r = m_fWorldRadius + pSphere->Radius;
    return dSq <= r * r;
}

_bool CBounding_Capsule::Intersects_Capsule(const CBounding_Capsule* pOther) const
{
    _float dSq = Sq_Dist_Segment_Segment(XMLoadFloat3(&m_vP0), XMLoadFloat3(&m_vP1),
        XMLoadFloat3(&pOther->m_vP0), XMLoadFloat3(&pOther->m_vP1));
    _float r = m_fWorldRadius + pOther->m_fWorldRadius;
    return dSq <= r * r;
}

// 박스류는 선분 샘플 + 구-박스 판정으로 근사(게임플레이 박스엔 충분)
_bool CBounding_Capsule::Intersects_OBB(const BoundingOrientedBox* pBox) const
{
    XMVECTOR A = XMLoadFloat3(&m_vP0), B = XMLoadFloat3(&m_vP1);
    const int N = 8;
    for (int i = 0; i <= N; ++i)
    {
        BoundingSphere s;
        XMStoreFloat3(&s.Center, XMVectorLerp(A, B, (_float)i / N));
        s.Radius = m_fWorldRadius;
        if (pBox->Intersects(s)) return true;
    }
    return false;
}

_bool CBounding_Capsule::Intersects_AABB(const BoundingBox* pBox) const
{
    XMVECTOR A = XMLoadFloat3(&m_vP0), B = XMLoadFloat3(&m_vP1);
    const int N = 8;
    for (int i = 0; i <= N; ++i)
    {
        BoundingSphere s;
        XMStoreFloat3(&s.Center, XMVectorLerp(A, B, (_float)i / N));
        s.Radius = m_fWorldRadius;
        if (pBox->Intersects(s)) return true;
    }
    return false;
}

_bool CBounding_Capsule::Intersect(COLLIDER eTargetType, CBounding* pBounding)
{
    m_isColl = false;
    switch (eTargetType)
    {
        case COLLIDER::AABB:
            m_isColl = Intersects_AABB(dynamic_cast<CBounding_AABB*>(pBounding)->Get_Desc()); break;
        case COLLIDER::OBB:
            m_isColl = Intersects_OBB(dynamic_cast<CBounding_OBB*>(pBounding)->Get_Desc()); break;
        case COLLIDER::SPHERE:
            m_isColl = Intersects_Sphere(dynamic_cast<CBounding_Sphere*>(pBounding)->Get_Desc()); break;
        case COLLIDER::CAPSULE:
            m_isColl = Intersects_Capsule(dynamic_cast<CBounding_Capsule*>(pBounding)); break;
    }
    return m_isColl;
}

#ifdef _DEBUG
HRESULT CBounding_Capsule::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
    XMVECTOR color = m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f)   // 충돌 중 = 빨강
        : XMVectorSet(0.f, 0.f, 1.f, 1.f);  // 평소 = 파랑

    Debug_DrawCapsule(pBatch, XMLoadFloat3(&m_vP0), XMLoadFloat3(&m_vP1), m_fWorldRadius, color);
    return S_OK;
}
#endif

CBounding_Capsule* CBounding_Capsule::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const
    CBounding::BOUNDING_DESC* pDesc)
{
    CBounding_Capsule* pInstance = new CBounding_Capsule(pDevice, pContext);
    if (FAILED(pInstance->Initialize(pDesc)))
    {
        MSG_BOX("Failed to Created : CBounding_Capsule");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBounding_Capsule::Free() { __super::Free(); }