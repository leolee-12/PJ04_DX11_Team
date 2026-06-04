#pragma once
#include "Base.h"

NS_BEGIN(Engine)

namespace Culling_Utility
{
	ENGINE_DLL _bool Is_ValidAABB(const _float3& vMin, const _float3& vMax);
	ENGINE_DLL BoundingBox Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax);
	ENGINE_DLL BoundingBox Make_DefaultAABB(_float fHalfExtent = 1.f);
	ENGINE_DLL _bool XM_CALLCONV Transform_AABB(const BoundingBox& LocalBounds, _fmatrix WorldMatrix, BoundingBox* pOutWorldBounds);
	ENGINE_DLL BoundingBox Merge_AABB(const BoundingBox& A, const BoundingBox& B);
	ENGINE_DLL void Expand_AABB(BoundingBox* pBounds, _float fMargin);
}

NS_END