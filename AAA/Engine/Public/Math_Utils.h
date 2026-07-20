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

    inline _bool Is_FiniteDouble(_double dValue)
    {
        return std::isfinite(dValue);
    }

    inline _float Sanitize_FiniteFloat(_float fValue, _float fDefault)
    {
        return Is_FiniteFloat(fValue) ? fValue : fDefault;
    }

    inline _float Sanitize_ClampedFloat(_float fValue, _float fDefault, _float fMin, _float fMax)
    {
        _float fResult = Sanitize_FiniteFloat(fValue, fDefault);
        Helper::FloatClamp(fResult, fMin, fMax);
        return fResult;
    }

    inline _float Sanitize_MinimumFloat(_float fValue, _float fDefault, _float fMin)
    {
        _float fResult = Sanitize_FiniteFloat(fValue, fDefault);
        if (fResult < fMin)
            fResult = fMin;

        return fResult;
    }

    inline _float Sanitize_MinimumAbsoluteFloat(_float fValue, _float fDefault, _float fMinimumAbsolute)
    {
        _float fResult = Sanitize_FiniteFloat(fValue, fDefault);
        const _float fSafeMinimumAbsolute = std::fabs(Sanitize_FiniteFloat(fMinimumAbsolute, 0.f));

        if (std::fabs(fResult) < fSafeMinimumAbsolute)
            fResult = fResult < 0.f ? -fSafeMinimumAbsolute : fSafeMinimumAbsolute;

        return fResult;
    }

    inline _float2 Sanitize_FiniteFloat2(const _float2& vValue, const _float2& vDefault)
    {
        return
        {
            Sanitize_FiniteFloat(vValue.x, vDefault.x),
            Sanitize_FiniteFloat(vValue.y, vDefault.y)
        };
    }

    inline _float4 Sanitize_FiniteFloat4(const _float4& vValue, const _float4& vDefault)
    {
        return
        {
            Sanitize_FiniteFloat(vValue.x, vDefault.x),
            Sanitize_FiniteFloat(vValue.y, vDefault.y),
            Sanitize_FiniteFloat(vValue.z, vDefault.z),
            Sanitize_FiniteFloat(vValue.w, vDefault.w)
        };
    }

    inline _double Wrap_FiniteDouble(_double dValue, _double dPeriod, _double dDefault = 0.0)
    {
        if (!Is_FiniteDouble(dValue) || !Is_FiniteDouble(dPeriod) || dPeriod <= 0.0)
            return dDefault;

        _double dResult = std::fmod(dValue, dPeriod);
        if (dResult < 0.0)
            dResult += dPeriod;

        return dResult;
    }

    inline _bool Is_ValidFloat3(const _float3& vValue)
    {
        return Is_FiniteFloat(vValue.x)
            && Is_FiniteFloat(vValue.y)
            && Is_FiniteFloat(vValue.z);
    }

    inline _float Abs_Float(_float fValue)
    {
        return std::fabs(fValue);
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
                if (std::fabs(A.m[iRow][iCol] - B.m[iRow][iCol]) > fEpsilon)
                    return false;
            }
        }

        return true;
    }
}

NS_END