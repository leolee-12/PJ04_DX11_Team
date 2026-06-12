#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCulling_Util final
{
public:
	static _bool Is_ValidAABB(const _float3& vMin, const _float3& vMax);
	static BoundingBox Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax);
	static BoundingBox Make_DefaultAABB(_float fHalfExtent = 1.f);
	static _bool XM_CALLCONV Transform_AABB(const BoundingBox& LocalBounds, _fmatrix WorldMatrix, BoundingBox* pOutWorldBounds);
	static BoundingBox Merge_AABB(const BoundingBox& A, const BoundingBox& B);
	static _bool Expand_AABB(BoundingBox* pBounds, _float fMargin);
	static _bool Check_CullByDistance(const BoundingBox& WorldBounds, const _float4& vCamPos, _float fCullDistance = 175.f);
};

NS_END