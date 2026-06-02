#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
bool g_bUseDiffuseTexture = { false };
float2 g_vDiffuseTiling = { 1.f, 1.f };
float2 g_vDiffuseOffset = { 0.f, 0.f };

Texture2D g_UnknownTexture;
bool g_bUseUnknownTexture = { false };
float2 g_vUnknownTiling = { 1.f, 1.f };
float2 g_vUnknownOffset = { 0.f, 0.f };

Texture2D g_Texture;
bool g_bUseTexture = { false };
float2 g_vTextureTiling = { 1.f, 1.f };
float2 g_vTextureOffset = { 0.f, 0.f };

Texture2D g_Mask;
bool g_bUseMask = { false };
float2 g_vMaskTiling = { 1.f, 1.f };
float2 g_vMaskOffset = { 0.f, 0.f };

float3 g_vColor = { 1.f, 1.f, 1.f };
float g_fAlpha = { 1.f };


struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};
    
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vPosition = mul(vPosition, g_ViewMatrix);
    vPosition = mul(vPosition, g_ProjMatrix);
    
    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}





struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};
    
PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
   
    Out.vColor = float4(1.f, 1.f, 1.f, 1.f);
    
    if (g_bUseTexture == true)
    {
        float2 vUV = g_vTextureOffset + In.vTexcoord * g_vTextureTiling;
        Out.vColor *= g_Texture.Sample(LinearSampler, vUV);
    }
    
    if (g_bUseMask == true)
    {
        float2 vUV = g_vMaskOffset + In.vTexcoord * g_vMaskTiling;
        Out.vColor *= g_Mask.Sample(LinearSampler, vUV);
    }
    
    if (g_bUseDiffuseTexture == true)
    {
        float2 vUV = g_vDiffuseOffset + In.vTexcoord * g_vDiffuseTiling;
        Out.vColor *= g_DiffuseTexture.Sample(LinearSampler, vUV);
    }
    
    if (g_bUseUnknownTexture == true)
    {
        float2 vUV = g_vUnknownOffset + In.vTexcoord * g_vUnknownTiling;
        Out.vColor *= g_UnknownTexture.Sample(LinearSampler, vUV);
    }
    
    Out.vColor.xyz *= g_vColor;          
    Out.vColor.a *= g_fAlpha;
    
    return Out;
}





technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass AlphaBlend
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }
}