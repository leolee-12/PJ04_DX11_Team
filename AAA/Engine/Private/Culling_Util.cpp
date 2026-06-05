#include "Culling_Util.h"

namespace
{
	_bool Is_FiniteFloat(_float fValue)
	{
		return std::isfinite(fValue);
	}
}

_bool CCulling_Util::Is_ValidAABB(const _float3& vMin, const _float3& vMax)
{
	return Is_FiniteFloat(vMin.x) && Is_FiniteFloat(vMin.y) && Is_FiniteFloat(vMin.z)
		&& Is_FiniteFloat(vMax.x) && Is_FiniteFloat(vMax.y) && Is_FiniteFloat(vMax.z)
		&& vMax.x >= vMin.x
		&& vMax.y >= vMin.y
		&& vMax.z >= vMin.z;
}

BoundingBox CCulling_Util::Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax)
{
	BoundingBox Bounds{};
	Bounds.Center = _float3(
		(vMin.x + vMax.x) * 0.5f,
		(vMin.y + vMax.y) * 0.5f,
		(vMin.z + vMax.z) * 0.5f);
	Bounds.Extents = _float3(
		(vMax.x - vMin.x) * 0.5f,
		(vMax.y - vMin.y) * 0.5f,
		(vMax.z - vMin.z) * 0.5f);
	return Bounds;
}

BoundingBox CCulling_Util::Make_DefaultAABB(_float fHalfExtent)
{
	if (!Is_FiniteFloat(fHalfExtent))
		fHalfExtent = 1.f;

	if (fHalfExtent < 0.f)
		fHalfExtent = -fHalfExtent;

	BoundingBox Bounds{};
	Bounds.Center = _float3(0.f, 0.f, 0.f);
	Bounds.Extents = _float3(fHalfExtent, fHalfExtent, fHalfExtent);
	return Bounds;
}

_bool XM_CALLCONV CCulling_Util::Transform_AABB(const BoundingBox& LocalBounds, _fmatrix WorldMatrix, BoundingBox* pOutWorldBounds)
{
	if (nullptr == pOutWorldBounds)
		return false;

	LocalBounds.Transform(*pOutWorldBounds, WorldMatrix);
	return true;
}

BoundingBox CCulling_Util::Merge_AABB(const BoundingBox& A, const BoundingBox& B)
{
	_float3 vCornersA[8] = {};
	_float3 vCornersB[8] = {};
	A.GetCorners(vCornersA);
	B.GetCorners(vCornersB);

	_float3 vMin = vCornersA[0];
	_float3 vMax = vCornersA[0];

	auto Accumulate = [&vMin, &vMax](const _float3& vPoint)
		{
			if (vPoint.x < vMin.x) vMin.x = vPoint.x;
			if (vPoint.y < vMin.y) vMin.y = vPoint.y;
			if (vPoint.z < vMin.z) vMin.z = vPoint.z;

			if (vPoint.x > vMax.x) vMax.x = vPoint.x;
			if (vPoint.y > vMax.y) vMax.y = vPoint.y;
			if (vPoint.z > vMax.z) vMax.z = vPoint.z;
		};

	for (_uint i = 1; i < 8; ++i)
		Accumulate(vCornersA[i]);

	for (_uint i = 0; i < 8; ++i)
		Accumulate(vCornersB[i]);

	return Make_AABB_FromMinMax(vMin, vMax);
}

_bool CCulling_Util::Expand_AABB(BoundingBox* pBounds, _float fMargin)
{
	if (nullptr == pBounds)
		return false;

	if (!Is_FiniteFloat(fMargin))
		return false;

	if (fMargin <= 0.f)
		return true;

	pBounds->Extents.x += fMargin;
	pBounds->Extents.y += fMargin;
	pBounds->Extents.z += fMargin;
	return true;
}