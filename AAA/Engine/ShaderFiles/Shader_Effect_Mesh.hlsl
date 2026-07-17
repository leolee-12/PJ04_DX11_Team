#include "Engine_Shader_Defines.hlsli"

#ifndef EFFECT_MESH_VERTEX_POSITION
#define EFFECT_MESH_VERTEX_POSITION(vPosition) (vPosition)
#endif

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);
uint g_iMaterialID = 0;

Texture2D g_DiffuseTexture;
bool g_bUseDiffuseTexture = { false };
bool g_bDiffuseColorToAlpha = { false };
bool g_bUseDiffuseUVEdgeFade = { false };
int g_iDiffuseUVEdgeFadeAxis = { 0 };
float g_fDiffuseUVEdgeFadeStartRange = { 0.1f };
float g_fDiffuseUVEdgeFadeEndRange = { 0.1f };
float g_fDiffuseUVEdgeFadePower = { 1.f };
float2 g_vDiffuseTiling = { 1.f, 1.f };
float2 g_vDiffuseOffset = { 0.f, 0.f };

Texture2D g_UnknownTexture;
bool g_bUseUnknownTexture = { false };
bool g_bUnknownColorToAlpha = { false };
bool g_bUseUnknownUVEdgeFade = { false };
int g_iUnknownUVEdgeFadeAxis = { 0 };
float g_fUnknownUVEdgeFadeStartRange = { 0.1f };
float g_fUnknownUVEdgeFadeEndRange = { 0.1f };
float g_fUnknownUVEdgeFadePower = { 1.f };
float2 g_vUnknownTiling = { 1.f, 1.f };
float2 g_vUnknownOffset = { 0.f, 0.f };

Texture2D g_NormalTexture;
bool g_bUseNormalTexture = { false };

Texture2D g_MRATexture;
bool g_bUseMRATexture = { false };

Texture2D g_Texture;
bool g_bUseTexture = { false };
bool g_bTextureColorToAlpha = { false };
bool g_bUseTextureUVEdgeFade = { false };
int g_iTextureUVEdgeFadeAxis = { 0 };
float g_fTextureUVEdgeFadeStartRange = { 0.1f };
float g_fTextureUVEdgeFadeEndRange = { 0.1f };
float g_fTextureUVEdgeFadePower = { 1.f };
float2 g_vTextureTiling = { 1.f, 1.f };
float2 g_vTextureOffset = { 0.f, 0.f };

Texture2D g_Mask;
bool g_bUseMask = { false };
float2 g_vMaskTiling = { 1.f, 1.f };
float2 g_vMaskOffset = { 0.f, 0.f };
int g_iMaskBlendMode = { 0 };
int g_iMaskChannel = { 0 };
bool g_bMaskInvert = { false };
float g_fMaskStrength = { 1.f };
bool g_bUseMaskUVDistortion = { false };
float2 g_vMaskUVDistortionStrength = { 0.f, 0.f };

float3 g_vColor = { 1.f, 1.f, 1.f };
float g_fAlpha = { 1.f };
float g_fEffectIntensity = { 1.f };

float3 g_vEffectMRA = { 0.f, 1.f, 1.f }; // metallic, roughness, ao
float g_fAlphaClip = { 0.01f };

float g_fCircleUVPI = { 3.141592f };

bool g_bUseCircleUVAnim_T = { false };
float g_fCircleUVRatio_T = { 1.f };
float g_fCircleUVStartDegree_T = { 0.f };
bool g_bCircleUVClockwise_T = { true };

bool g_bUseCircleUVAnim_M = { false };
float g_fCircleUVRatio_M = { 1.f };
float g_fCircleUVStartDegree_M = { 0.f };
bool g_bCircleUVClockwise_M = { true };

bool g_bUseCircleUVAnim_D = { false };
float g_fCircleUVRatio_D = { 1.f };
float g_fCircleUVStartDegree_D = { 0.f };
bool g_bCircleUVClockwise_D = { true };

bool g_bUseCircleUVAnim_U = { false };
float g_fCircleUVRatio_U = { 1.f };
float g_fCircleUVStartDegree_U = { 0.f };
bool g_bCircleUVClockwise_U = { true };

bool g_bUseLinearUVAnim_T = { false };
float g_fLinearUVRatio_T = { 1.f };
int g_iLinearUVAxis_T = { 0 };
bool g_bLinearUVReverse_T = { false };

bool g_bUseLinearUVAnim_M = { false };
float g_fLinearUVRatio_M = { 1.f };
int g_iLinearUVAxis_M = { 0 };
bool g_bLinearUVReverse_M = { false };

bool g_bUseLinearUVAnim_D = { false };
float g_fLinearUVRatio_D = { 1.f };
int g_iLinearUVAxis_D = { 0 };
bool g_bLinearUVReverse_D = { false };

bool g_bUseLinearUVAnim_U = { false };
float g_fLinearUVRatio_U = { 1.f };
int g_iLinearUVAxis_U = { 0 };
bool g_bLinearUVReverse_U = { false };

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
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float3 vLocalPosition = EFFECT_MESH_VERTEX_POSITION(In.vPosition);
    float4 vWorld = mul(float4(vLocalPosition, 1.f), g_WorldMatrix);
    float4 vView = mul(vWorld, g_ViewMatrix);
    float4 vProj = mul(vView, g_ProjMatrix);

    Out.vPosition = vProj;
    Out.vTexcoord = In.vTexcoord;
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vProjPos = vProj;
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix).xyz);
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix).xyz);

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vNormal : NORMAL;
    float4 vProjPos : TEXCOORD1;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
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
    float4 vEmissive : SV_TARGET4;
    float4 vGeoNormal : SV_TARGET5;
    uint   vMaterialID : SV_TARGET6;
};

void Apply_CircleUVAnim(float2 vTexcoord, bool bUse, float fRatio, float fStartDegree, bool bClockwise)
{
    if (bUse == false)
        return;

    float2 vCenterUV = vTexcoord - float2(0.5f, 0.5f);
    float fRadian = atan2(vCenterUV.x, -vCenterUV.y);

    if (fRadian < 0.f)
        fRadian += 2.f * g_fCircleUVPI;

    float fAngleRatio = fRadian / (2.f * g_fCircleUVPI);
    float fStartRatio = frac(fStartDegree / 360.f);
    fAngleRatio = frac(fAngleRatio - fStartRatio + 1.f);

    if (bClockwise == false)
        fAngleRatio = frac(1.f - fAngleRatio);

    if (fAngleRatio >= saturate(fRatio))
        discard;
}

void Apply_LinearUVAnim(float2 vTexcoord, bool bUse, float fRatio, int iAxis, bool bReverse)
{
    if (bUse == false)
        return;

    float fAxis = iAxis == 1 ? vTexcoord.y : vTexcoord.x;

    if (bReverse == true)
        fAxis = 1.f - fAxis;

    if (fAxis > saturate(fRatio))
        discard;
}

float4 ResolveMaskValue(float4 vMaskSample)
{
    if (g_iMaskChannel == 1)
        vMaskSample = vMaskSample.rrrr;
    else if (g_iMaskChannel == 2)
        vMaskSample = vMaskSample.gggg;
    else if (g_iMaskChannel == 3)
        vMaskSample = vMaskSample.bbbb;
    else if (g_iMaskChannel == 4)
        vMaskSample = vMaskSample.aaaa;

    if (g_bMaskInvert == true)
        vMaskSample = 1.f - vMaskSample;

    return vMaskSample;
}

float4 ApplyMaskBlend(float4 vColor, float4 vMaskValue)
{
    float fStrength = max(g_fMaskStrength, 0.f);

    if (g_iMaskBlendMode == 1)
    {
        vColor.rgb += vColor.rgb * vMaskValue.rgb * fStrength;
        return vColor;
    }
    if (g_iMaskBlendMode == 2)
        return max(vColor - vMaskValue * fStrength, 0.f);
    if (g_iMaskBlendMode == 3)
        return lerp(vColor, vMaskValue, saturate(fStrength));

    return vColor * lerp(float4(1.f, 1.f, 1.f, 1.f), vMaskValue, saturate(fStrength));
}

float4 ApplyColorToAlpha(float4 vTextureValue, bool bUseColorToAlpha)
{
    if (bUseColorToAlpha == true)
    {
        vTextureValue.a *= dot(saturate(vTextureValue.rgb), float3(0.299f, 0.587f, 0.114f));
        vTextureValue.rgb = float3(1.f, 1.f, 1.f);
    }

    return vTextureValue;
}

float ComputeUVEdgeFade1D(float fCoord, float fStartRange, float fEndRange)
{
    fCoord = saturate(fCoord);
    fStartRange = saturate(fStartRange);
    fEndRange = saturate(fEndRange);

    float fStartFade = fStartRange > 0.0001f
        ? smoothstep(0.f, fStartRange, fCoord) : 1.f;
    float fEndFade = fEndRange > 0.0001f
        ? smoothstep(0.f, fEndRange, 1.f - fCoord) : 1.f;

    return fStartFade * fEndFade;
}

float4 ApplyUVEdgeFade(
    float4 vTextureValue, float2 vTexcoord, bool bUseEdgeFade,
    int iAxis, float fStartRange, float fEndRange, float fPower)
{
    if (bUseEdgeFade == false)
        return vTextureValue;

    float fFadeX = ComputeUVEdgeFade1D(vTexcoord.x, fStartRange, fEndRange);
    float fFadeY = ComputeUVEdgeFade1D(vTexcoord.y, fStartRange, fEndRange);
    float fFade = iAxis == 0 ? fFadeX : (iAxis == 1 ? fFadeY : fFadeX * fFadeY);

    vTextureValue.a *= pow(saturate(fFade), clamp(fPower, 0.1f, 8.f));
    return vTextureValue;
}

float4 ComposeEffectColor(float2 vTexcoord, SamplerState EffectSampler)
{
    float4 vColor = float4(1.f, 1.f, 1.f, 1.f);
    float4 vMaskValue = float4(1.f, 1.f, 1.f, 1.f);
    float2 vUVDistortion = float2(0.f, 0.f);

    if (g_bUseMask == true)
    {
        Apply_CircleUVAnim(vTexcoord, g_bUseCircleUVAnim_M, g_fCircleUVRatio_M, g_fCircleUVStartDegree_M, g_bCircleUVClockwise_M);
        Apply_LinearUVAnim(vTexcoord, g_bUseLinearUVAnim_M, g_fLinearUVRatio_M, g_iLinearUVAxis_M, g_bLinearUVReverse_M);

        float2 vUV = g_vMaskOffset + vTexcoord * g_vMaskTiling;
        vMaskValue = ResolveMaskValue(g_Mask.Sample(EffectSampler, vUV));

        if (g_bUseMaskUVDistortion == true)
            vUVDistortion = (vMaskValue.rr * 2.f - 1.f) * g_vMaskUVDistortionStrength;
    }

    if (g_bUseTexture == true)
    {
        Apply_CircleUVAnim(vTexcoord, g_bUseCircleUVAnim_T, g_fCircleUVRatio_T, g_fCircleUVStartDegree_T, g_bCircleUVClockwise_T);
        Apply_LinearUVAnim(vTexcoord, g_bUseLinearUVAnim_T, g_fLinearUVRatio_T, g_iLinearUVAxis_T, g_bLinearUVReverse_T);

        float2 vUV = g_vTextureOffset + vTexcoord * g_vTextureTiling + vUVDistortion;
        float4 vTextureValue = ApplyColorToAlpha(
            g_Texture.Sample(EffectSampler, vUV), g_bTextureColorToAlpha);
        vTextureValue = ApplyUVEdgeFade(
            vTextureValue, vTexcoord, g_bUseTextureUVEdgeFade,
            g_iTextureUVEdgeFadeAxis, g_fTextureUVEdgeFadeStartRange,
            g_fTextureUVEdgeFadeEndRange, g_fTextureUVEdgeFadePower);
        vColor *= vTextureValue;
    }

    if (g_bUseDiffuseTexture == true)
    {
        Apply_CircleUVAnim(vTexcoord, g_bUseCircleUVAnim_D, g_fCircleUVRatio_D, g_fCircleUVStartDegree_D, g_bCircleUVClockwise_D);
        Apply_LinearUVAnim(vTexcoord, g_bUseLinearUVAnim_D, g_fLinearUVRatio_D, g_iLinearUVAxis_D, g_bLinearUVReverse_D);
        
        float2 vUV = g_vDiffuseOffset + vTexcoord * g_vDiffuseTiling + vUVDistortion;
        float4 vDiffuseValue = ApplyColorToAlpha(
            g_DiffuseTexture.Sample(EffectSampler, vUV), g_bDiffuseColorToAlpha);
        vDiffuseValue = ApplyUVEdgeFade(
            vDiffuseValue, vTexcoord, g_bUseDiffuseUVEdgeFade,
            g_iDiffuseUVEdgeFadeAxis, g_fDiffuseUVEdgeFadeStartRange,
            g_fDiffuseUVEdgeFadeEndRange, g_fDiffuseUVEdgeFadePower);
        vColor *= vDiffuseValue;
    }

    if (g_bUseUnknownTexture == true)
    {
        Apply_CircleUVAnim(vTexcoord, g_bUseCircleUVAnim_U, g_fCircleUVRatio_U, g_fCircleUVStartDegree_U, g_bCircleUVClockwise_U);
        Apply_LinearUVAnim(vTexcoord, g_bUseLinearUVAnim_U, g_fLinearUVRatio_U, g_iLinearUVAxis_U, g_bLinearUVReverse_U);

        float2 vUV = g_vUnknownOffset + vTexcoord * g_vUnknownTiling + vUVDistortion;
        float4 vUnknownValue = ApplyColorToAlpha(
            g_UnknownTexture.Sample(EffectSampler, vUV), g_bUnknownColorToAlpha);
        vUnknownValue = ApplyUVEdgeFade(
            vUnknownValue, vTexcoord, g_bUseUnknownUVEdgeFade,
            g_iUnknownUVEdgeFadeAxis, g_fUnknownUVEdgeFadeStartRange,
            g_fUnknownUVEdgeFadeEndRange, g_fUnknownUVEdgeFadePower);
        vColor *= vUnknownValue;
    }

    if (g_bUseMask == true)
        vColor = ApplyMaskBlend(vColor, vMaskValue);

    vColor.rgb *= g_vColor * g_fEffectIntensity;
    vColor.a *= g_fAlpha;

    return vColor;
}

float4 ComposeEffectColor_Linear(float2 vTexcoord)
{
    return ComposeEffectColor(vTexcoord, LinearSampler);
}

float4 ComposeEffectColor_Mirror(float2 vTexcoord)
{
    return ComposeEffectColor(vTexcoord, MirrorSampler);
}

PS_GBUFFER_OUT PS_GBUFFER(PS_IN In)
{
    PS_GBUFFER_OUT Out;

    float4 vColor = ComposeEffectColor_Linear(In.vTexcoord);

    if (vColor.a <= g_fAlphaClip)
        discard;

    float3 vGeometryNormal = normalize(In.vNormal.xyz);
    float3 vNormal = vGeometryNormal;

    if (g_bUseNormalTexture == true)
    {
        float3 vTangent = normalize(In.vTangent);
        float3 vBinormal = normalize(In.vBinormal);
        float3x3 TBN = float3x3(vTangent, vBinormal, vGeometryNormal);

        float2 vNormalRG = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
        float3 vTangentNormal = float3(
            vNormalRG, sqrt(saturate(1.f - dot(vNormalRG, vNormalRG))));
        vNormal = normalize(mul(vTangentNormal, TBN));
    }

    float3 vMRA = g_vEffectMRA;
    if (g_bUseMRATexture == true)
        vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

    Out.vDiffuse = float4(vColor.rgb, 1.f);
    Out.vNormal = float4(vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
    Out.vMaterialID = g_iMaterialID;
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vColor.a, 1.f);
    Out.vGeoNormal = float4(vGeometryNormal * 0.5f + 0.5f, 0.f);

    return Out;
}

PS_COLOR_OUT PS_MAIN(PS_IN In)
{
    PS_COLOR_OUT Out;
    Out.vColor = ComposeEffectColor_Linear(In.vTexcoord);
    Out.vColor.rgb += g_vEmissiveColor.rgb * Out.vColor.a;
    return Out;
}

PS_COLOR_OUT PS_MAIN_MIRROR(PS_IN In)
{
    PS_COLOR_OUT Out;
    Out.vColor = ComposeEffectColor_Mirror(In.vTexcoord);
    Out.vColor.rgb += g_vEmissiveColor.rgb * Out.vColor.a;
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass //0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GBUFFER()));
    }

    pass AlphaBlend //1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass Additive //2 
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass DefaultPass_Mirror //3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GBUFFER()));
    }

    pass AlphaBlend_Mirror //4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass Additive_Mirror //5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass DefaultPass_DepthIgnore //6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GBUFFER()));
    }

    pass AlphaBlend_DepthIgnore //7
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass Additive_DepthIgnore //8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass DefaultPass_Mirror_DepthIgnore //9
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GBUFFER()));
    }

    pass AlphaBlend_Mirror_DepthIgnore //10
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass Additive_Mirror_DepthIgnore //11
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
}
