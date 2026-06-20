#pragma once
#include "DebugDraw.h"   // PrimitiveBatch / VertexPositionColor / DirectXMath 제공

NS_BEGIN(Engine)

// p0,p1 = 두 반구 중심(월드), r = 반지름
// radial = 원주 분할(촘촘함), rings = 반구 호 분할
inline void Debug_DrawCapsule(
    DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* pBatch,
    DirectX::FXMVECTOR p0, DirectX::FXMVECTOR p1, float r,
    DirectX::FXMVECTOR color, int radial = 16, int rings = 4)
{
    using namespace DirectX;

    XMVECTOR axis = p1 - p0;
    float len = XMVectorGetX(XMVector3Length(axis));
    axis = (len > 1e-6f) ? XMVector3Normalize(axis) : XMVectorSet(0.f, 1.f, 0.f, 0.f);

    XMVECTOR up = (fabsf(XMVectorGetY(axis)) > 0.99f) ? XMVectorSet(1.f, 0.f, 0.f, 0.f)
        : XMVectorSet(0.f, 1.f, 0.f, 0.f);
    XMVECTOR u = XMVector3Normalize(XMVector3Cross(axis, up));
    XMVECTOR v = XMVector3Cross(axis, u);

    auto Line = [&](FXMVECTOR a, FXMVECTOR b)
        { pBatch->DrawLine(VertexPositionColor(a, color), VertexPositionColor(b, color)); };
    auto Dir = [&](float ang) { return u * cosf(ang) + v * sinf(ang); };

    // 원기둥 세로선 + 양 끝 링
    XMVECTOR prev0 = p0 + Dir(0.f) * r;
    XMVECTOR prev1 = p1 + Dir(0.f) * r;
    Line(prev0, prev1);
    for (int i = 1; i <= radial; ++i)
    {
        XMVECTOR d = Dir(XM_2PI * i / radial);
        XMVECTOR a0 = p0 + d * r;
        XMVECTOR a1 = p1 + d * r;
        Line(a0, a1);          // 세로선
        Line(prev0, a0);       // p0 링
        Line(prev1, a1);       // p1 링
        prev0 = a0; prev1 = a1;
    }

    // 반구 돔(경도 호): p1쪽 +axis, p0쪽 -axis 로 휨
    for (int j = 0; j < radial; ++j)
    {
        XMVECTOR d = Dir(XM_2PI * j / radial);
        XMVECTOR pt = p1 + d * r;
        XMVECTOR pb = p0 + d * r;
        for (int k = 1; k <= rings; ++k)
        {
            float t = XM_PIDIV2 * k / rings;
            float cs = cosf(t), sn = sinf(t);
            XMVECTOR nt = p1 + d * (r * cs) + axis * (r * sn);
            XMVECTOR nb = p0 + d * (r * cs) + axis * (-r * sn);
            Line(pt, nt); pt = nt;
            Line(pb, nb); pb = nb;
        }
    }
}

NS_END