#pragma once
#include "Engine_Defines.h"

#include <cmath>

NS_BEGIN(Engine)

namespace MathUtils
{
    inline _bool Is_FiniteFloat(_float fValue)
    {
        return std::isfinite(fValue);
    }

    inline _bool Is_ValidFloat3(const _float3& vValue)
    {
        return Is_FiniteFloat(vValue.x)
            && Is_FiniteFloat(vValue.y)
            && Is_FiniteFloat(vValue.z);
    }



    inline _float Abs_Float(_float fValue)
    {
        return fValue < 0.f ? -fValue : fValue;
    }

    inline _float3 Abs_Float3(const _float3& vValue)
    {
        return { Abs_Float(vValue.x), Abs_Float(vValue.y), Abs_Float(vValue.z) };
    }



    inline _bool Is_NearlyEqualFloat4x4(const _float4x4& A, const _float4x4& B, _float fEpsilon = Helper::fEpsilon)
    {
        for (_uint iRow = 0; iRow < 4; ++iRow)
        {
            for (_uint iCol = 0; iCol < 4; ++iCol)
            {
                if (fabsf(A.m[iRow][iCol] - B.m[iRow][iCol]) > fEpsilon)
                    return false;
            }
        }

        return true;
    }
}

NS_END