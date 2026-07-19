#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);
float3 g_vColor = float3(1.f, 1.f, 1.f);
float g_fAlpha = 1.f;
float g_fEffectIntensity = 1.f;

Texture2D g_Texture;
bool g_bUseTexture = false;
float2 g_vTextureTiling = float2(1.f, 1.f);
float2 g_vTextureOffset = float2(0.f, 0.f);
float g_fTextureUVRotationDegree = 0.f;

Texture2D g_Mask;
bool g_bUseMask = false;
float2 g_vMaskTiling = float2(1.f, 1.f);
float2 g_vMaskOffset = float2(0.f, 0.f);
float g_fMaskUVRotationDegree = 0.f;
int g_iMaskBlendMode = 0;
int g_iMaskChannel = 0;
bool g_bMaskInvert = false;
float g_fMaskStrength = 1.f;
bool g_bUseMaskUVDistortion = false;
float2 g_vMaskUVDistortionStrength = float2(0.f, 0.f);

float g_fTrailLifeTime = 0.3f;
float g_fTrailEdgeSoftness = 0.15f;
float g_fTrailHeadFadeRatio = 0.f;
float g_fTrailTailFadeRatio = 0.35f;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fAge : TEXCOORD1;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fAge : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    float4 vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vPosition = mul(vPosition, g_ViewMatrix);
    Out.vPosition = mul(vPosition, g_ProjMatrix);
    Out.vTexcoord = In.vTexcoord;
    Out.fAge = In.fAge;
    return Out;
}

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

float4 ResolveMaskValue(float4 vValue)
{
    if (g_iMaskChannel == 1) vValue = vValue.rrrr;
    else if (g_iMaskChannel == 2) vValue = vValue.gggg;
    else if (g_iMaskChannel == 3) vValue = vValue.bbbb;
    else if (g_iMaskChannel == 4) vValue = vValue.aaaa;
    if (g_bMaskInvert == true) vValue = 1.f - vValue;
    return vValue;
}

float4 ApplyMaskBlend(float4 vColor, float4 vMaskValue)
{
    const float fStrength = max(g_fMaskStrength, 0.f);
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

float2 RotateEffectUV(float2 vTexcoord, float fDegree)
{
    float fSin = 0.f;
    float fCos = 1.f;
    sincos(radians(fDegree), fSin, fCos);

    float2 vCentered = vTexcoord - float2(0.5f, 0.5f);
    return float2(
        vCentered.x * fCos - vCentered.y * fSin,
        vCentered.x * fSin + vCentered.y * fCos) + float2(0.5f, 0.5f);
}

float ComputeTrailAlpha(VS_OUT In)
{
    const float fLifeRatio = saturate(In.fAge / max(g_fTrailLifeTime, 0.0001f));

    float fHeadAlpha = 1.f;
    if (g_fTrailHeadFadeRatio > 0.0001f)
        fHeadAlpha = smoothstep(0.f, g_fTrailHeadFadeRatio, fLifeRatio);

    float fTailAlpha = 1.f;
    if (g_fTrailTailFadeRatio > 0.0001f)
        fTailAlpha = 1.f - smoothstep(
            1.f - g_fTrailTailFadeRatio, 1.f, fLifeRatio);

    float fEdgeAlpha = 1.f;
    if (g_fTrailEdgeSoftness > 0.0001f)
    {
        const float fEdgeDistance = abs(In.vTexcoord.y * 2.f - 1.f);
        fEdgeAlpha = 1.f - smoothstep(
            1.f - g_fTrailEdgeSoftness, 1.f, fEdgeDistance);
    }

    return fHeadAlpha * fTailAlpha * fEdgeAlpha;
}

PS_OUT ComposeTrailColor(VS_OUT In, SamplerState effectSampler)
{
    PS_OUT Out;
    float4 vColor = float4(1.f, 1.f, 1.f, 1.f);
    float4 vMaskValue = float4(1.f, 1.f, 1.f, 1.f);
    float2 vDistortion = float2(0.f, 0.f);

    if (g_bUseMask == true)
    {
        const float2 vRotatedMaskUV = RotateEffectUV(In.vTexcoord, g_fMaskUVRotationDegree);
        const float2 vMaskUV = g_vMaskOffset + vRotatedMaskUV * g_vMaskTiling;
        vMaskValue = ResolveMaskValue(g_Mask.Sample(effectSampler, vMaskUV));
        if (g_bUseMaskUVDistortion == true)
            vDistortion = (vMaskValue.rr * 2.f - 1.f) * g_vMaskUVDistortionStrength;
    }

    if (g_bUseTexture == true)
    {
        const float2 vRotatedTextureUV = RotateEffectUV(In.vTexcoord, g_fTextureUVRotationDegree);
        const float2 vTextureUV =
            g_vTextureOffset + vRotatedTextureUV * g_vTextureTiling + vDistortion;
        vColor *= g_Texture.Sample(effectSampler, vTextureUV);
    }

    if (g_bUseMask == true)
        vColor = ApplyMaskBlend(vColor, vMaskValue);

    vColor.rgb *= g_vColor * g_fEffectIntensity;
    vColor.a *= g_fAlpha * ComputeTrailAlpha(In);
    vColor.rgb += g_vEmissiveColor.rgb * vColor.a;
    Out.vColor = vColor;
    return Out;
}

PS_OUT PS_MAIN(VS_OUT In)
{
    return ComposeTrailColor(In, LinearSampler);
}

PS_OUT PS_MAIN_MIRROR(VS_OUT In)
{
    return ComposeTrailColor(In, MirrorSampler);
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
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
    pass Additive
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass Max
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Max, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }
    pass DefaultPass_Mirror
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
    pass AlphaBlend_Mirror
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
    pass Additive_Mirror
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass Max_Mirror
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Max, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
    pass DefaultPass_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }
    pass AlphaBlend_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }
    pass Additive_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass Max_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Max, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }
    pass DefaultPass_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
    pass AlphaBlend_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
    pass Additive_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass Max_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Max, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
}
