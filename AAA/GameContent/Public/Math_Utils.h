#pragma once
#include "GameContent_Defines.h"

#include <cmath>

NS_BEGIN(Client)

inline _bool Is_NearlyEqualFloat4x4(const _float4x4& A, const _float4x4& B, _float fEpsilon = Engine::Helper::fEpsilon)
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

NS_END