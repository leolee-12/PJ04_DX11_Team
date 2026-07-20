#include "Bounding_Torus.h"
#include "Bounding_Sphere.h"
#include "Bounding_Capsule.h"

CBounding_Torus::CBounding_Torus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBounding(pDevice, pContext) {
}

HRESULT CBounding_Torus::Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc)
{
    auto pDesc = static_cast<const BOUNDING_TORUS_DESC*>(pBoundingDesc);

    m_vLocalCenter = pDesc->vCenter;
    m_fRing = pDesc->fRingRadius;
    m_fTube = pDesc->fTubeRadius;

    _matrix matRot = XMMatrixRotationRollPitchYaw(
        pDesc->vRadians.x, pDesc->vRadians.y, pDesc->vRadians.z);
    XMStoreFloat3(&m_vLocalAxis, XMVector3TransformNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f), matRot));

    return S_OK;
}

void CBounding_Torus::Update(_fmatrix TransformMatrix)
{
    XMStoreFloat3(&m_vWorldCenter,
        XMVector3TransformCoord(XMLoadFloat3(&m_vLocalCenter), TransformMatrix));
    XMStoreFloat3(&m_vWorldAxis,
        XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&m_vLocalAxis), TransformMatrix)));

    const _float fScale = XMVectorGetX(XMVector3Length(TransformMatrix.r[0]));   // 균등 스케일 가정
    m_fWorldRing = m_fRing * fScale;
    m_fWorldTube = m_fTube * fScale;
}

void CBounding_Torus::Reset_Desc(const CBounding::BOUNDING_DESC* pDesc)
{
    Initialize(pDesc);
}

_float CBounding_Torus::Distance_ToSurface(_fvector vPoint) const
{
    _vector vAxis = XMLoadFloat3(&m_vWorldAxis);
    _vector v = vPoint - XMLoadFloat3(&m_vWorldCenter);

    const _float h = XMVectorGetX(XMVector3Dot(v, vAxis));          // 축 방향 높이
    const _float fRad = XMVectorGetX(XMVector3Length(v - vAxis * h)); // 축에서의 반경 거리

    const _float dx = fRad - m_fWorldRing;
    return sqrtf(dx * dx + h * h) - m_fWorldTube;
}

_bool CBounding_Torus::Intersects_Sphere(const BoundingSphere* pSphere) const
{
    _vector vC = XMVectorSet(pSphere->Center.x, pSphere->Center.y, pSphere->Center.z, 1.f);
    return Distance_ToSurface(vC) <= pSphere->Radius;
}

_bool CBounding_Torus::Intersects_Capsule(const CBounding_Capsule* pCapsule) const
{
    _float3 vP0, vP1;
    pCapsule->Get_Segment(vP0, vP1);
    const _float fR = pCapsule->Get_Radius();

    // 세그먼트를 샘플링해서 구 판정 (게임플레이용 근사)
    constexpr _int SAMPLES = 5;
    _vector vA = XMLoadFloat3(&vP0);
    _vector vB = XMLoadFloat3(&vP1);
    for (_int i = 0; i < SAMPLES; ++i)
    {
        const _float t = (_float)i / (SAMPLES - 1);
        if (Distance_ToSurface(XMVectorLerp(vA, vB, t)) <= fR)
            return true;
    }
    return false;
}

_bool CBounding_Torus::Intersect(COLLIDER eTargetType, CBounding* pBounding)
{
    switch (eTargetType)
    {
        case COLLIDER::SPHERE:
            return Intersects_Sphere(static_cast<CBounding_Sphere*>(pBounding)->Get_Desc());
        case COLLIDER::CAPSULE:
            return Intersects_Capsule(static_cast<CBounding_Capsule*>(pBounding));
        case COLLIDER::TORUS:
        {
            // 내 링을 샘플링해서 상대 토러스와 거리 판정
            auto pOther = static_cast<CBounding_Torus*>(pBounding);
            _vector vAxis = XMLoadFloat3(&m_vWorldAxis);
            _vector vC = XMLoadFloat3(&m_vWorldCenter);
            _vector vX = XMVector3Normalize(XMVector3Orthogonal(vAxis));
            _vector vZ = XMVector3Cross(vAxis, vX);

            constexpr _int SAMPLES = 16;
            for (_int i = 0; i < SAMPLES; ++i)
            {
                const _float a = XM_2PI * i / SAMPLES;
                _vector vP = vC + (vX * cosf(a) + vZ * sinf(a)) * m_fWorldRing;
                if (pOther->Distance_ToSurface(vP) <= m_fWorldTube)
                    return true;
            }
            return false;
        }
        default:
            return false;   // AABB/OBB 페어는 미지원 (필요 시 링 샘플링으로 추가)
    }
}

#ifdef _DEBUG
HRESULT CBounding_Torus::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
    const _float4 vColor = Get_DebugRenderColor();
    XMVECTORF32 color{ vColor.x, vColor.y, vColor.z, vColor.w };

    _vector vAxis = XMLoadFloat3(&m_vWorldAxis);
    _vector vC = XMLoadFloat3(&m_vWorldCenter);
    _vector vX = XMVector3Normalize(XMVector3Orthogonal(vAxis));
    _vector vZ = XMVector3Cross(vAxis, vX);

    auto DrawRing = [&](_float fRadius, _float fHeight)
        {
            constexpr _int SEG = 32;
            _vector vBase = vC + vAxis * fHeight;
            for (_int i = 0; i < SEG; ++i)
            {
                const _float a0 = XM_2PI * i / SEG;
                const _float a1 = XM_2PI * (i + 1) / SEG;
                _vector p0 = vBase + (vX * cosf(a0) + vZ * sinf(a0)) * fRadius;
                _vector p1 = vBase + (vX * cosf(a1) + vZ * sinf(a1)) * fRadius;
                pBatch->DrawLine(VertexPositionColor(p0, color), VertexPositionColor(p1, color));
            }
        };

    DrawRing(m_fWorldRing + m_fWorldTube, 0.f);   // 바깥 링
    DrawRing(m_fWorldRing - m_fWorldTube, 0.f);   // 안쪽 링
    DrawRing(m_fWorldRing, m_fWorldTube);        // 윗면 링
    DrawRing(m_fWorldRing, -m_fWorldTube);       // 아랫면 링

    return S_OK;
}
#endif

CBounding_Torus* CBounding_Torus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CBounding::BOUNDING_DESC* pDesc)
{
    CBounding_Torus* pInstance = new CBounding_Torus(pDevice, pContext);
    if (FAILED(pInstance->Initialize(pDesc)))
    {
        MSG_BOX("Failed to Created : CBounding_Torus");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBounding_Torus::Free() { __super::Free(); }