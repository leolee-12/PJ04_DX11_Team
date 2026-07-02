bool g_bUseRingDeform = { false };
float g_fRingHeight = { 0.f };
float g_fRingStartRadius = { 0.166626f };
float g_fRingEndRadius = { 0.5f };

float3 GetRingVertexPosition(float3 vPosition)
{
    if (g_bUseRingDeform == false)
        return vPosition;

    float fRadius = length(vPosition.xy);
    float fRadiusRange = max(g_fRingEndRadius - g_fRingStartRadius, 0.00001f);
    float fDeformRatio = saturate((fRadius - g_fRingStartRadius) / fRadiusRange);

    vPosition.z += g_fRingHeight * fDeformRatio;

    return vPosition;
}

#define EFFECT_MESH_VERTEX_POSITION(vPosition) GetRingVertexPosition(vPosition)
#include "Shader_Effect_Mesh.hlsl"
