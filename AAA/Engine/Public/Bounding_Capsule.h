#pragma once
#include "Bounding.h"

NS_BEGIN(Engine)
class CBounding_AABB; class CBounding_OBB; class CBounding_Sphere;

class CBounding_Capsule final : public CBounding
{
public:
    typedef struct tagBoundingCapsuleDesc : public CBounding::BOUNDING_DESC
    {
        _float  fRadius;     // 캡슐 반지름
        _float  fHeight;     // 두 반구중심 사이 거리(원기둥 길이)
        _float3 vRadians;    // 로컬 방향(기본 +Y up)
    }BOUNDING_CAPSULE_DESC;

private:
    CBounding_Capsule(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CBounding_Capsule() = default;

public:
    virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc) override;
    virtual void    Update(_fmatrix TransformMatrix) override;
    virtual void    Reset_Desc(const CBounding::BOUNDING_DESC* pDesc) override;
    virtual _bool   Intersect(COLLIDER eTargetType, CBounding* pBounding) override;

    // 다른 bounding의 switch가 호출하는 진입점(캡슐 수학을 여기 한 곳에)
    _bool Intersects_Sphere(const BoundingSphere* pSphere) const;
    _bool Intersects_AABB(const BoundingBox* pBox) const;
    _bool Intersects_OBB(const BoundingOrientedBox* pBox) const;
    _bool Intersects_Capsule(const CBounding_Capsule* pOther) const;

    void   Get_Segment(_float3& vP0, _float3& vP1) const { vP0 = m_vP0; vP1 = m_vP1; }
    _float Get_Radius() const { return m_fWorldRadius; }

#ifdef _DEBUG
    virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) override;
#endif

private:
    _float3 m_vLocalP0 = {};            // 로컬 반구중심
    _float3 m_vLocalP1 = {};
    _float  m_fRadius = { 0.5f };
    _float3 m_vP0 = {};                 // 월드 반구중심
    _float3 m_vP1 = {};
    _float  m_fWorldRadius = { 0.5f };

public:
    static CBounding_Capsule* Create(ID3D11Device*, ID3D11DeviceContext*, const CBounding::BOUNDING_DESC*);
    virtual void Free() override;
};
NS_END