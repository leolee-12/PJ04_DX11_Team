#include "Picking_Utils.h"

_bool CPicking_Utils::Pick_RayToPlane(_fvector vOrigin, _fvector vDir, _float3* pOutHit, _float fPlaneY, _float fFar)
{
    // 1e-6f        = 0.000001 일반 앱실론
    // 1e-7f        = 0.0000001 엄격한 앱실론
    // FLT_EPSILON  = 약 1.19e-7 float 최소 정밀도 (표준 라이브러리 제공)
	// t = (PlaneY - OriginY) / DirY
	_float fOriginY = XMVectorGetY(vOrigin);
    _float fDirY = XMVectorGetY(vDir);

    static constexpr _float fEpsilon = 1e-6f;  

    if (fabsf(fDirY) < fEpsilon)
        return false; // 평면과 평행

    XMVECTOR plane = XMVectorSet(0.f, 1.f, 0.f, -fPlaneY); // ax+by+cz+d=0 → d=-fPlaneY

    XMVECTOR v2 = XMVectorAdd(vOrigin, XMVectorScale(vDir, fFar));

    _vector vHit = XMPlaneIntersectLine(plane, vOrigin, v2);
    XMStoreFloat3(pOutHit, vHit);
    return true;
}

bool CPicking_Utils::RayToMesh(_fvector vOrigin, _fvector vDir, const _float3* pVertices, _uint iVertexCount, const _uint* pIndices, _uint iIndexCount, _float3* pOutHit, float* pOutDist)
{
    if (!pVertices || !pIndices || iIndexCount == 0)
        return false;

    _float minT = FLT_MAX;
    _bool  bHit = false;

    for (_uint i = 0; i + 2 < iIndexCount; i += 3)
    {
        _uint i0 = pIndices[i];
        _uint i1 = pIndices[i + 1];
        _uint i2 = pIndices[i + 2];

        if (i0 >= iVertexCount || i1 >= iVertexCount || i2 >= iVertexCount)
            continue;

        XMVECTOR v0 = XMLoadFloat3(&pVertices[i0]);
        XMVECTOR v1 = XMLoadFloat3(&pVertices[i1]);
        XMVECTOR v2 = XMLoadFloat3(&pVertices[i2]);

        float t;
        if (TriangleTests::Intersects(vOrigin, vDir, v0, v1, v2, t))
        {
            if (t < minT)
            {
                minT = t;
                bHit = true;
            }
        }
    }

    if (bHit)
    {
        XMStoreFloat3(pOutHit, XMVectorAdd(vOrigin, XMVectorScale(vDir, minT)));
        if (pOutDist) *pOutDist = minT;
    }

    return bHit;
}
