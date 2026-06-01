#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

bool g_bUseDiffuseTexture = { false };
Texture2D g_DiffuseTexture;

bool g_bUseUnknownTexture = { false };
Texture2D g_UnknownTexture;

bool g_bUseTexture = { false };
Texture2D g_Texture;

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
    
    if (g_bUseDiffuseTexture == true)
        Out.vColor *= g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    
    if (g_bUseUnknownTexture == true)
        Out.vColor *= g_UnknownTexture.Sample(LinearSampler, In.vTexcoord);
    
    if (g_bUseTexture == true)
        Out.vColor *= g_Texture.Sample(LinearSampler, In.vTexcoord);         
    
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