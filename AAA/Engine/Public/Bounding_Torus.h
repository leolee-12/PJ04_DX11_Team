#pragma once
#include "Bounding.h"

NS_BEGIN(Engine)
class CBounding_Capsule;

class CBounding_Torus final : public CBounding
{
public:
    typedef struct tagBoundingTorusDesc : public CBounding::BOUNDING_DESC
    {
        _float  fRingRadius;   // 링(대) 반지름: 중심~튜브 중심
        _float  fTubeRadius;   // 튜브(소) 반지름: 도넛 두께의 절반
        _float3 vRadians;      // 축 기울기 (기본 +Y up)
        _float  fArcDeg;
    }BOUNDING_TORUS_DESC;

private:
    CBounding_Torus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CBounding_Torus() = default;

public:
    virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc) override;
    virtual void    Update(_fmatrix TransformMatrix) override;
    virtual void    Reset_Desc(const CBounding::BOUNDING_DESC* pDesc) override;
    virtual _bool   Intersect(COLLIDER eTargetType, CBounding* pBounding) override;

    _bool  Intersects_Sphere(const BoundingSphere* pSphere) const;
    _bool  Intersects_Capsule(const CBounding_Capsule* pCapsule) const;
    _float Distance_ToSurface(_fvector vPoint) const;   // 표면까지 부호 거리(내부=음수)

#ifdef _DEBUG
    virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) override;
#endif

private:
    _float3 m_vLocalCenter = {};
    _float3 m_vLocalAxis = { 0.f, 1.f, 0.f };
    _float  m_fRing = { 1.f };
    _float  m_fTube = { 0.25f };

    _float3 m_vWorldCenter = {};
    _float3 m_vWorldAxis = { 0.f, 1.f, 0.f };
    _float  m_fWorldRing = { 1.f };
    _float  m_fWorldTube = { 0.25f };

    _float  m_fArcRad = { XM_2PI };          // 호 각도(라디안)
    _float3 m_vLocalFwd = { 0.f, 0.f, 1.f }; // 호 중앙 방향 (로컬 +Z)
    _float3 m_vWorldFwd = { 0.f, 0.f, 1.f };

public:
    static CBounding_Torus* Create(ID3D11Device*, ID3D11DeviceContext*, const CBounding::BOUNDING_DESC*);
    virtual void Free() override;
};
NS_END