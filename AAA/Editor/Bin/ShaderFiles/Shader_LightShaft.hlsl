#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float3 g_vColor = float3(1.f, 0.5f, 0.15f);
float g_fAlpha = 1.f;
float g_fIntensity = 1.f;

struct VS_IN
{
    float3 vPos : POSITION;
    float3 vNor : NORMAL;
    float2 vTex : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPos : SV_POSITION;
    float2 vTex : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 vWorld = mul(float4(In.vPos, 1.f), g_WorldMatrix);
    float4 vView = mul(vWorld, g_ViewMatrix);
    Out.vPos = mul(vView, g_ProjMatrix);
    Out.vTex = In.vTex;

    return Out;
}

float4 PS_MAIN(VS_OUT In) : SV_TARGET0
{
    return float4(g_vColor * g_fIntensity, g_fAlpha);
}

// TODO(CHJ): fresnel(view-angle) + depth intersection fade + base-to-apex gradient for a volumetric look.
technique11 DefaultTechnique
{
    pass LightShaft // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }
}
