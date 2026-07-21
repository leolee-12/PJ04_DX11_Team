#pragma once
#include "Math_Utils.h"

NS_BEGIN(Engine)

namespace GeometryUtils
{
	inline _float3 Flip_Axis(STATE eAxis, const _float3& vPivot)
	{
		switch (eAxis)
		{
		case STATE::RIGHT:  return { -vPivot.x, vPivot.y, vPivot.z };
		case STATE::UP:     return { vPivot.x, -vPivot.y, vPivot.z };
		case STATE::LOOK:   return { vPivot.x, vPivot.y, -vPivot.z };
		default:            return _float3{};
		}
		return _float3{};
	}

	inline _bool Is_ValidAABB(const _float3& vMin, const _float3& vMax)
	{
		return MathUtils::Is_ValidFloat3(vMin)
			&& MathUtils::Is_ValidFloat3(vMax)
			&& vMax.x >= vMin.x
			&& vMax.y >= vMin.y
			&& vMax.z >= vMin.z;
	}

	inline _bool Is_ValidAABB(const BoundingBox& Bounds)
	{
		return MathUtils::Is_ValidFloat3(Bounds.Center)
			&& MathUtils::Is_ValidFloat3(Bounds.Extents)
			&& Bounds.Extents.x >= 0.f
			&& Bounds.Extents.y >= 0.f
			&& Bounds.Extents.z >= 0.f;
	}

	inline _bool Has_UsableSize(const _float3& vSize, _float fMinAxis = 0.001f)
	{
		const _float3 vAbsSize = MathUtils::Abs_Float3(vSize);
		return vAbsSize.x > fMinAxis
			&& vAbsSize.y > fMinAxis
			&& vAbsSize.z > fMinAxis;
	}

	inline _float3 Make_AbsSize(const _float3& vSize)
	{
		return MathUtils::Abs_Float3(vSize);
	}

	inline _float3 Make_HalfExtentsFromSize(const _float3& vSize)
	{
		const _float3 vAbsSize = MathUtils::Abs_Float3(vSize);
		return { vAbsSize.x * 0.5f, vAbsSize.y * 0.5f, vAbsSize.z * 0.5f };
	}

	inline _bool Try_Make_YAxisCapsuleFromSize(const _float3& vSize, _float* pOutRadius, _float* pOutHalfHeight)
	{
		if (nullptr == pOutRadius || nullptr == pOutHalfHeight || !Has_UsableSize(vSize))
			return false;

		const _float3 vAbsSize = Make_AbsSize(vSize);
		const _float fRadius = max(vAbsSize.x, vAbsSize.z) * 0.5f;
		const _float fHalfHeight = max(vAbsSize.y * 0.5f - fRadius, 0.f);

		if (!MathUtils::Is_FiniteFloat(fRadius) || fRadius <= 0.f)
			return false;

		if (!MathUtils::Is_FiniteFloat(fHalfHeight) || fHalfHeight < 0.f)
			return false;

		*pOutRadius = fRadius;
		*pOutHalfHeight = fHalfHeight;
		return true;
	}

	inline BoundingBox Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax)
	{
		BoundingBox Bounds{};
		Bounds.Center = {
				(vMin.x + vMax.x) * 0.5f,
				(vMin.y + vMax.y) * 0.5f,
				(vMin.z + vMax.z) * 0.5f
		};
		Bounds.Extents = {
				(vMax.x - vMin.x) * 0.5f,
				(vMax.y - vMin.y) * 0.5f,
				(vMax.z - vMin.z) * 0.5f
		};
		return Bounds;
	}

	inline BoundingBox Make_DefaultAABB(_float fHalfExtent = 1.f)
	{
		if (!MathUtils::Is_FiniteFloat(fHalfExtent))
			fHalfExtent = 1.f;

		if (fHalfExtent < 0.f)
			fHalfExtent = -fHalfExtent;

		return BoundingBox(_float3(0.f, 0.f, 0.f), _float3(fHalfExtent, fHalfExtent, fHalfExtent));
	}

	inline _bool XM_CALLCONV Transform_AABB(const BoundingBox& LocalBounds, _fmatrix WorldMatrix, BoundingBox* pOutWorldBounds)
	{
		if (nullptr == pOutWorldBounds)
			return false;

		LocalBounds.Transform(*pOutWorldBounds, WorldMatrix);
		return true;
	}

	inline BoundingBox Merge_AABB(const BoundingBox& A, const BoundingBox& B)
	{
		BoundingBox Merged{};
		BoundingBox::CreateMerged(Merged, A, B);
		return Merged;
	}

	inline _bool Expand_AABB(BoundingBox* pBounds, _float fMargin)
	{
		if (nullptr == pBounds || !MathUtils::Is_FiniteFloat(fMargin))
			return false;

		// AABB 축소 미지원, 음수 마진은 무시
		if (fMargin <= 0.f)
			return true;

		pBounds->Extents.x += fMargin;
		pBounds->Extents.y += fMargin;
		pBounds->Extents.z += fMargin;
		return true;
	}
}

NS_END