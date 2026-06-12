#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

bool g_bAlphaTest = false;
int g_iInvertMask = 0;

int g_iFrame = { 0 }; // SpriteAnim 슬라이스 선택
int g_iMaskChannel = { 3 }; // 0 : R, 1 : G, 2 : B, 3 : A
int g_iFillDirection = { 0 }; // 0 : Left To Right / 1 : Right to Left

float g_fAlpha = { 1.f };
float g_fTestAlpha = { 0.f };

float g_fMaskPower = { 1.f };
float g_fEffectIntensity = { 1.f };
float g_fRevealProgress = { 1.f };
float g_fRevealSoftness = { 0.05f };

float g_fFillRatio = { 1.f };
float g_fFillSoftness = { 0.f };

float2 g_vEffectTiling = float2(1.f, 1.f);
float2 g_vEffectOffset = float2(0.f, 0.f);

float4 g_vColor = { 1.f, 1.f, 1.f, 1.f };
float4 g_vUVTransform = { 1.f, 1.f, 0.f, 0.f };

Texture2D g_Texture;
Texture2D g_EffectTexture;
Texture2D g_MaskTexture;

Texture2DArray g_TextureArray;              // SpriteAnim용

float2 ApplyUVTransform(float2 uv)
{
    return uv * g_vUVTransform.xy + g_vUVTransform.zw;
}

// SampleMask 헬퍼
float ExtractMask(float4 fMaskTex)
{
    float mask = fMaskTex.a;

    switch (g_iMaskChannel)
    {
        case 0:
            mask = fMaskTex.r;
            break;
        case 1:
            mask = fMaskTex.g;
            break;
        case 2:
            mask = fMaskTex.b;
            break;
        case 3:
            mask = fMaskTex.a;
            break;
    }

    if (g_iInvertMask != 0)
        mask = 1.f - mask;

    mask = saturate(mask);
    mask = pow(mask, max(g_fMaskPower, 0.0001f));

    return mask;
}

float ComputeGaugeVisible(float2 uv)
{
    float fRatio = saturate(g_fFillRatio);

    // 0.f / 1.f 일 때 아래의 연산을 안하기 위함
    if (fRatio <= 0.f)
        return 0.f;

    if (fRatio >= 1.f)
        return 1.f;

    float fAxis = uv.x;

    if (g_iFillDirection == 1)
        fAxis = 1.f - fAxis;

    float fSoftness = max(g_fFillSoftness, 0.f);

    if (fSoftness <= 0.f)
        return fAxis <= fRatio ? 1.f : 0.f;

    return 1.f - smoothstep(fRatio, min(fRatio + fSoftness, 1.f), fAxis);
}

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
    float4 vPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vPos = mul(vPos, g_ViewMatrix);
    vPos = mul(vPos, g_ProjMatrix);

    Out.vPosition = vPos;
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
    float2 uv = ApplyUVTransform(In.vTexcoord);
    Out.vColor = g_Texture.Sample(UISampler, uv);
    Out.vColor *= g_vColor;
    Out.vColor.a *= g_fAlpha;

    if (Out.vColor.a <= 0.f)
        discard;

    if (g_bAlphaTest && Out.vColor.a <= g_fTestAlpha)
        discard;

    return Out;
}

PS_OUT PS_SPRITEANIM(PS_IN In)
{
    PS_OUT Out;
    Out.vColor = g_TextureArray.Sample(UISampler, float3(In.vTexcoord, (float) g_iFrame));
    Out.vColor *= g_vColor;
    Out.vColor.a *= g_fAlpha;

    if (Out.vColor.a <= 0.f)
        discard;
    if (g_bAlphaTest && Out.vColor.a <= g_fTestAlpha)
        discard;

    return Out;
}

PS_OUT PS_MASKED_COLOR(PS_IN In)
{
    PS_OUT Out;

    // Mask Texture에서 알파값 추출
    float maskAlpha = ExtractMask(g_MaskTexture.Sample(UISampler, In.vTexcoord));

    Out.vColor.rgb = g_vColor.rgb * g_fEffectIntensity;
    Out.vColor.a = g_vColor.a * g_fAlpha * maskAlpha;

    if (Out.vColor.a <= 0.f)
        discard;

    return Out;
}

PS_OUT PS_MASKED_TEXTURE(PS_IN In)
{
    PS_OUT Out;

    float maskAlpha = ExtractMask(g_MaskTexture.Sample(UISampler, In.vTexcoord));

    float2 effectUV = In.vTexcoord * g_vEffectTiling + g_vEffectOffset; // 정적 텍스처는 g_vEffectOffset을 고정값으로  / 동적 텍스처는 g_vEffectOffset += scrollDir * time;
    float4 effect = g_EffectTexture.Sample(LinearSampler, effectUV);

    Out.vColor.rgb = effect.rgb * g_vColor.rgb * g_fEffectIntensity;
    Out.vColor.a = effect.a * g_vColor.a * g_fAlpha * maskAlpha;

    if (Out.vColor.a <= 0.f)
        discard;

    return Out;
}

PS_OUT PS_BRUSH_REVEAL(PS_IN In)
{
    PS_OUT Out;

    float revealMask = ExtractMask(g_MaskTexture.Sample(UISampler, In.vTexcoord));

    float progress = saturate(g_fRevealProgress);
    float softness = max(g_fRevealSoftness, 0.0001f);
    float visible = smoothstep(revealMask - softness, revealMask + softness, progress);

    float4 tex = g_EffectTexture.Sample(UISampler, In.vTexcoord);

    Out.vColor.rgb = tex.rgb * g_vColor.rgb * g_fEffectIntensity;
    //Out.vColor.a = tex.a * g_vColor.a * g_fAlpha * visible;
    Out.vColor.a = tex.a * g_vColor.a * g_fAlpha * visible;

    if (Out.vColor.a <= 0.f)
        discard;

    return Out;
}

PS_OUT PS_GAUGE_FILL_COLOR(PS_IN In)
{
    PS_OUT Out;

    float fVisible = ComputeGaugeVisible(In.vTexcoord);
    if (fVisible <= 0.f)
        discard;

    float fMask = ExtractMask(g_Texture.Sample(UISampler, In.vTexcoord));

    Out.vColor.rgb = g_vColor.rgb;
    Out.vColor.a = g_vColor.a * g_fAlpha * fMask * fVisible;

    if (Out.vColor.a <= 0.f)
        discard;

    return Out;
}

PS_OUT PS_GAUGE_FILL_TEXTURE(PS_IN In)
{
    PS_OUT Out;

    float fVisible = ComputeGaugeVisible(In.vTexcoord);
    if (fVisible <= 0.f)
        discard;

    float4 fTex = g_Texture.Sample(UISampler, In.vTexcoord);

    Out.vColor.rgb = fTex.rgb * g_vColor.rgb;
    Out.vColor.a = fTex.a * g_vColor.a * g_fAlpha * fVisible;

    if (Out.vColor.a <= 0.f)
        discard;

    return Out;
}

technique11 DefaultTechnique
{
    pass UI // pass 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass SpriteAnim // pass 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_SPRITEANIM()));
    }

    pass MaskedTexture // pass 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MASKED_TEXTURE()));
    }

    pass MaskedColor    // pass 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MASKED_COLOR()));
    }

    pass MaskedAdd // pass 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MASKED_TEXTURE()));
    }

    pass BrushReveal // pass 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_BRUSH_REVEAL()));
    }

    pass GaugeFillColor // pass 6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GAUGE_FILL_COLOR()));
    }

    pass GaugeFillTexture // pass 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_GAUGE_FILL_TEXTURE()));
    }
}