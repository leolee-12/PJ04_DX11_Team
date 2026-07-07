#pragma once
#include "Math_Utils.h"

NS_BEGIN(Engine)

namespace GeometryUtils
{
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

    inline _bool Expand_AABB(BoundingBox* pBounds, _float fMargin)
    {
        if (nullptr == pBounds || !MathUtils::Is_FiniteFloat(fMargin))
            return false;

        if (fMargin <= 0.f)
            return true;

        pBounds->Extents.x += fMargin;
        pBounds->Extents.y += fMargin;
        pBounds->Extents.z += fMargin;
        return true;
    }
}

NS_END