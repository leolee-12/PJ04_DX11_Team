#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)

class ENGINE_DLL CPicking_Utils final
{
public:
    static _bool Pick_RayToPlane(
        _fvector vOrigin,
        _fvector vDir,
        _float3* pOutHit,
        _float  fPlaneY,
        _float fFar
    );

    static bool RayToMesh(
        _fvector       vOrigin,
        _fvector       vDir,
        const _float3* pVertices, _uint iVertexCount,
        const _uint* pIndices, _uint iIndexCount,
        _float3* pOutHit,
        float* pOutDist = nullptr
    );
};

NS_END

