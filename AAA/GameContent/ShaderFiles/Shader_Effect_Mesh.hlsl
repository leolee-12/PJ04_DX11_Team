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

float3 g_vEffectMRA = { 0.f, 1.f, 1.f }; // metallic, roughness, ao
float g_fAlphaClip = { 0.01f };

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
    float4 vNormal : NORMAL;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 vWorld = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    float4 vView = mul(vWorld, g_ViewMatrix);
    float4 vProj = mul(vView, g_ProjMatrix);

    Out.vPosition = vProj;
    Out.vTexcoord = In.vTexcoord;
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vProjPos = vProj;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vNormal : NORMAL;
    float4 vProjPos : TEXCOORD1;
};

struct PS_COLOR_OUT
{
    float4 vColor : SV_TARGET0;
};

struct PS_GBUFFER_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vMRA : SV_TARGET3;
};

float4 ComposeEffectColor_Linear(float2 vTexcoord)
{
    float4 vColor = float4(1.f, 1.f, 1.f, 1.f);

    if (g_bUseTexture == true)
    {
        float2 vUV = g_vTextureOffset + vTexcoord * g_vTextureTiling;
        vColor *= g_Texture.Sample(LinearSampler, vUV);
    }

    if (g_bUseMask == true)
    {
        float2 vUV = g_vMaskOffset + vTexcoord * g_vMaskTiling;
        vColor *= g_Mask.Sample(LinearSampler, vUV);
    }

    if (g_bUseDiffuseTexture == true)
    {
        float2 vUV = g_vDiffuseOffset + vTexcoord * g_vDiffuseTiling;
        vColor *= g_DiffuseTexture.Sample(LinearSampler, vUV);
    }

    if (g_bUseUnknownTexture == true)
    {
        float2 vUV = g_vUnknownOffset + vTexcoord * g_vUnknownTiling;
        vColor *= g_UnknownTexture.Sample(LinearSampler, vUV);
    }

    vColor.rgb *= g_vColor;
    vColor.a *= g_fAlpha;

    return vColor;
}

float4 ComposeEffectColor_Mirror(float2 vTexcoord)
{
    float4 vColor = float4(1.f, 1.f, 1.f, 1.f);

    if (g_bUseTexture == true)
    {
        float2 vUV = g_vTextureOffset + vTexcoord * g_vTextureTiling;
        vColor *= g_Texture.Sample(MirrorSampler, vUV);
    }

    if (g_bUseMask == true)
    {
        float2 vUV = g_vMaskOffset + vTexcoord * g_vMaskTiling;
        vColor *= g_Mask.Sample(MirrorSampler, vUV);
    }

    if (g_bUseDiffuseTexture == true)
    {
        float2 vUV = g_vDiffuseOffset + vTexcoord * g_vDiffuseTiling;
        vColor *= g_DiffuseTexture.Sample(MirrorSampler, vUV);
    }

    if (g_bUseUnknownTexture == true)
    {
        float2 vUV = g_vUnknownOffset + vTexcoord * g_vUnknownTiling;
        vColor *= g_UnknownTexture.Sample(MirrorSampler, vUV);
    }

    vColor.rgb *= g_vColor;
    vColor.a *= g_fAlpha;

    return vColor;
}

PS_GBUFFER_OUT PS_GBUFFER(PS_IN In)
{
    PS_GBUFFER_OUT Out;

    float4 vColor = ComposeEffectColor_Linear(In.vTexcoord);

    if (vColor.a <= g_fAlphaClip)
        discard;

    float3 vNormal = normalize(In.vNormal.xyz);

    Out.vDiffuse = float4(vColor.rgb, 1.f);
    Out.vNormal = float4(vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(g_vEffectMRA, 1.f);

    return Out;
}

PS_COLOR_OUT PS_MAIN(PS_IN In)
{
    PS_COLOR_OUT Out;
    Out.vColor = ComposeEffectColor_Linear(In.vTexcoord);
    return Out;
}

PS_COLOR_OUT PS_MAIN_MIRROR(PS_IN In)
{
    PS_COLOR_OUT Out;
    Out.vColor = ComposeEffectColor_Mirror(In.vTexcoord);
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass //0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GBUFFER()));
    }

    pass AlphaBlend //1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass Additive //2 
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass DefaultPass_Mirror //3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GBUFFER()));
    }

    pass AlphaBlend_Mirror //4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass Additive_Mirror //5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
}