#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
bool g_bBillboard = { false };

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);
bool g_bWriteEmissiveGBuffer = { false };
uint g_iMaterialID = 0;

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

bool g_bSpriteAniTexture = { false };
float2 g_vSpriteAniTexUV = { 0.f, 0.f };
float2 g_vSpriteAniTexSize = { 1.f, 1.f };

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

bool g_bSpriteAniMask = { false };
float2 g_vSpriteAniMaskUV = { 0.f, 0.f };
float2 g_vSpriteAniMaskSize = { 1.f, 1.f };

bool g_bFlipX = { false };
bool g_bFlipY = { false };
float3 g_vColor = { 1.f, 1.f, 1.f };
float g_fAlpha = { 1 };
float g_fEffectIntensity = { 1.f };

bool g_bAlphaTest = { false };
float g_fTestAlpha = { 0.f };

float g_fUVCutRight = { 0.f };
float g_fUVCutLeft = { 0.f };

float g_fUVCutTop = { 0.f };
float g_fUVCutBottom = { 0.f };

float g_fRoll = { 0.f };

float g_fDissolve;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 vPosition;

    if (g_bBillboard == true)
    {
        float3 vScale;
        vScale.x = length(mul(float4(1.f, 0.f, 0.f, 0.f), g_WorldMatrix).xyz);
        vScale.y = length(mul(float4(0.f, 1.f, 0.f, 0.f), g_WorldMatrix).xyz);
        vScale.z = length(mul(float4(0.f, 0.f, 1.f, 0.f), g_WorldMatrix).xyz);

        float2 vLocal = In.vPosition.xy * vScale.xy; // 스케일 적용
        float s = sin(g_fRoll);
        float c = cos(g_fRoll);
        float2 vRolled = float2(vLocal.x * c - vLocal.y * s,
                                  vLocal.x * s + vLocal.y * c); // 화면 롤

        vPosition = mul(float4(0.f, 0.f, 0.f, 1.f), g_WorldMatrix);
        vPosition = mul(vPosition, g_ViewMatrix);
        vPosition.xyz += float3(vRolled, In.vPosition.z * vScale.z);
        vPosition = mul(vPosition, g_ProjMatrix);
    }
    else
    {
        vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
        vPosition = mul(vPosition, g_ViewMatrix);
        vPosition = mul(vPosition, g_ProjMatrix);
    }
    
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

struct PS_GBUFFER_OUT
{
    float4 vColor : SV_TARGET0;
    float4 vEmissive : SV_TARGET4;
};

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

float4 ComposeEffectBaseColor(PS_IN In, SamplerState EffectSampler)
{
    if (In.vTexcoord.x > (1.f - g_fUVCutRight) || In.vTexcoord.x < g_fUVCutLeft)
        discard;
    
    if (In.vTexcoord.y > (1.f - g_fUVCutBottom) || In.vTexcoord.y < g_fUVCutTop)
        discard;
    
    if (g_bFlipX == 1)
        In.vTexcoord.x = -In.vTexcoord.x + 1.f;
    
    if (g_bFlipY == 1)
        In.vTexcoord.y = -In.vTexcoord.y + 1.f;
    
    float4 vColor = float4(1.f, 1.f, 1.f, 1.f);

    float4 vMaskValue = float4(1.f, 1.f, 1.f, 1.f);
    float2 vUVDistortion = float2(0.f, 0.f);

    if (g_bUseMask == true)
    {
        float2 vUV = float2(0.f, 0.f);

        if (g_bSpriteAniMask == true)
            vUV = In.vTexcoord * g_vSpriteAniMaskSize + g_vSpriteAniMaskUV;
        else
            vUV = g_vMaskOffset + In.vTexcoord * g_vMaskTiling;

        vMaskValue = ResolveMaskValue(g_Mask.Sample(EffectSampler, vUV));

        if (g_bUseMaskUVDistortion == true)
            vUVDistortion = (vMaskValue.rr * 2.f - 1.f) * g_vMaskUVDistortionStrength;
    }
    
    if (g_bUseTexture == true)
    {
        float2 vUV = float2(0.f, 0.f);
        
        if (g_bSpriteAniTexture == true)
            vUV = In.vTexcoord * g_vSpriteAniTexSize + g_vSpriteAniTexUV + vUVDistortion;
        else
            vUV = g_vTextureOffset + In.vTexcoord * g_vTextureTiling + vUVDistortion;
        
        float4 vTextureValue = ApplyColorToAlpha(
            g_Texture.Sample(EffectSampler, vUV), g_bTextureColorToAlpha);
        vTextureValue = ApplyUVEdgeFade(
            vTextureValue, In.vTexcoord, g_bUseTextureUVEdgeFade,
            g_iTextureUVEdgeFadeAxis, g_fTextureUVEdgeFadeStartRange,
            g_fTextureUVEdgeFadeEndRange, g_fTextureUVEdgeFadePower);
        vColor *= vTextureValue;
    }
    
    if (g_bUseMask == true)
        vColor = ApplyMaskBlend(vColor, vMaskValue);
    
    vColor.xyz *= g_vColor * g_fEffectIntensity;
    vColor.a *= g_fAlpha;
    
    if (g_bAlphaTest == true && vColor.a <= g_fTestAlpha)
        discard;
    
    return vColor;
}

PS_OUT ComposeEffectColor(PS_IN In, SamplerState EffectSampler)
{
    PS_OUT Out;
    Out.vColor = ComposeEffectBaseColor(In, EffectSampler);
    Out.vColor.rgb += g_vEmissiveColor.rgb * Out.vColor.a;
    return Out;
}

PS_GBUFFER_OUT ComposeDefaultEffectColor(PS_IN In, SamplerState EffectSampler)
{
    PS_GBUFFER_OUT Out;
    Out.vColor = ComposeEffectBaseColor(In, EffectSampler);

    const float3 vEmissive = g_vEmissiveColor.rgb * Out.vColor.a;
    if (g_bWriteEmissiveGBuffer == true)
    {
        Out.vEmissive = float4(vEmissive, 1.f);
    }
    else
    {
        Out.vColor.rgb += vEmissive;
        Out.vEmissive = float4(0.f, 0.f, 0.f, 0.f);
    }

    return Out;
}




PS_OUT PS_MAIN(PS_IN In)
{
    return ComposeEffectColor(In, LinearSampler);
}

PS_OUT PS_MAIN_MIRROR(PS_IN In)
{
    return ComposeEffectColor(In, MirrorSampler);
}

PS_GBUFFER_OUT PS_DEFAULT(PS_IN In)
{
    return ComposeDefaultEffectColor(In, LinearSampler);
}

PS_GBUFFER_OUT PS_DEFAULT_MIRROR(PS_IN In)
{
    return ComposeDefaultEffectColor(In, MirrorSampler);
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
        SetPixelShader(CompileShader(ps_5_0, PS_DEFAULT()));
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

    pass DefaultPass_Mirror
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_DEFAULT_MIRROR()));
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

    pass DefaultPass_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_DEFAULT()));
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

    pass DefaultPass_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_DEFAULT_MIRROR()));
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
}
