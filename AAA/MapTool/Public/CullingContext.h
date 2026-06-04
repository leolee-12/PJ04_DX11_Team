#pragma once

#include "MapTool_Defines.h"
#include "Base.h"

NS_BEGIN(MapTool)

class CCullingContext final : public CBase
{
private:
	CCullingContext() = default;
	virtual ~CCullingContext() = default;

public:
	HRESULT Initialize();

	_bool Update_FromViewProj(const _float4x4* pView, const _float4x4* pProj);
	_bool XM_CALLCONV Update_FromViewProj(_fmatrix ViewMatrix, _fmatrix ProjMatrix);

	_bool XM_CALLCONV IsIn_WorldSpace(_fvector vWorldPos, _float fRange = 0.f) const;
	_bool IsIn_WorldSpace_AABB(const BoundingBox& WorldBounds) const;
	_bool Should_CullAABB(_bool bEnableCulling, const BoundingBox& WorldBounds) const;

	const BoundingFrustum& Get_WorldFrustum() const { return m_WorldFrustum; }
	_bool Has_Frustum() const { return m_bHasFrustum; }

public:
	static _bool Is_ValidAABB(const _float3& vMin, const _float3& vMax);
	static BoundingBox Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax);
	static BoundingBox Make_DefaultAABB();
	static _bool XM_CALLCONV Transform_AABB(const BoundingBox& LocalBounds, _fmatrix WorldMatrix, BoundingBox* pOutWorldBounds);
	static BoundingBox Merge_AABB(const BoundingBox& A, const BoundingBox& B);

public:
	static CCullingContext* Create();

private:
	BoundingFrustum m_WorldFrustum = {};
	_bool			m_bHasFrustum = { false };

protected:
	virtual void Free() override;
};

NS_END
