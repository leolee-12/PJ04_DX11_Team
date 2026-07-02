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
float g_fCapFracU = { 0.2f };

float g_fGhostRatio = { 0.f }; // 직전체력 비율(0이면 일반 게이지)
float g_fGhostAlpha = { 1.f }; // 노랑 깜빡임 알파
float4 g_vGhostColor = { 1.f, 0.92f, 0.2f, 1.f }; // 직전체력 잔상 색(노랑)

float g_fFlashFront = { 1.f }; 
float g_fFlashBlend = { 0.f }; 
float4 g_vFlashColor = { 1.f, 1.f, 1.f, 1.f };

float2 g_vEffectTiling = float2(1.f, 1.f);
float2 g_vEffectOffset = float2(0.f, 0.f);

float4 g_vColor = { 1.f, 1.f, 1.f, 1.f };
float4 g_vUVTransform = { 1.f, 1.f, 0.f, 0.f };

Texture2D g_Texture;
Texture2D g_EffectTexture;
Texture2D g_MaskTexture;

Texture2DArray g_TextureArray;              // SpriteAnim용
bool g_bUseTexture = true; // 커튼: 텍스처 있으면 곱하고, 없으면 단색

float Remap_BarU(float u)
{
    float c = min(g_fCapFracU, 0.5f);
    float result = 1.0f;

    if (u < c)
        result = u / c;
    else if (u > 1.0f - c)
        result = (1.0f - u) / c;

    return result;
}

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

PS_OUT PS_ALPHAERASE(PS_IN In)
{
    PS_OUT Out;
    float a = g_Texture.Sample(UISampler, In.vTexcoord).a * g_fAlpha; // 별=1, 바깥=0
    Out.vColor = float4(0.f, 0.f, 0.f, a); // RGB는 BS_AlphaErase에서 무시, a가 마스크
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

    float fAxis = In.vTexcoord.x;
    if (g_iFillDirection == 1)
        fAxis = 1.f - fAxis;

    float fFill = saturate(g_fFillRatio);
    float fGhost = max(fFill, saturate(g_fGhostRatio)); // 잔상은 항상 현재 이상

    if (fAxis > fGhost)                                   // 직전체력 밖 = 빈 공간
        discard;

    float2 shapeUV = float2(Remap_BarU(In.vTexcoord.x), In.vTexcoord.y);
    float4 vColor = g_Texture.Sample(UISampler, shapeUV);

      // 현재(핑크) ↔ 직전구간(노랑) 경계. g_fFillSoftness로 부드럽게(0이면 칼같이)
    float fSoft = max(g_fFillSoftness, 0.f);
    float fPink = 1.f - smoothstep(fFill, fFill + fSoft, fAxis); // 1=핑크, 0=노랑

    float3 col = lerp(g_vGhostColor.rgb, g_vColor.rgb, fPink);
    float a = lerp(g_vGhostColor.a * g_fGhostAlpha,
                        g_vColor.a * g_fAlpha,
                        fPink);

    Out.vColor.rgb = col;
    Out.vColor.a = a * vColor.a;

    if (Out.vColor.a <= 0.f)
        discard;

    return Out;
}

PS_OUT PS_GAUGE_FILL_TEXTURE(PS_IN In)
{
    PS_OUT Out;

    float fAxis = In.vTexcoord.x;
    if (g_iFillDirection == 1)
        fAxis = 1.f - fAxis;

    float fFill = saturate(g_fFillRatio);
    float fGhost = max(fFill, saturate(g_fGhostRatio));

    if (fAxis > fGhost)                                   
        discard;

    float4 fTex = g_Texture.Sample(UISampler, In.vTexcoord);

    float fSoft = max(g_fFillSoftness, 0.f);
    float fFilled = 1.f - smoothstep(fFill, fFill + fSoft, fAxis);

    float3 col = lerp(g_vGhostColor.rgb, fTex.rgb * g_vColor.rgb, fFilled);
    float a = fTex.a * g_fAlpha *
                   lerp(g_vGhostColor.a * g_fGhostAlpha, g_vColor.a, fFilled);

    Out.vColor.rgb = col;
    Out.vColor.a = a;

    if (Out.vColor.a <= 0.f)
        discard;

    return Out;
}

PS_OUT PS_CURTAINFILL(PS_IN In)
{
    PS_OUT Out;
    float4 col = g_vColor; // 단색 베이스
    if (g_bUseTexture)
        col *= g_Texture.Sample(UISampler, ApplyUVTransform(In.vTexcoord));
    col.a *= g_fAlpha;

    if (col.a <= 0.f)
        discard;

    Out.vColor = col;
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

    pass CurtainFill // pass 8 : 커튼 RT 일반 텍스처(배경/구멍위 이미지), 알파 기록
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_CurtainOver, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_CURTAINFILL())); // 기존 PS 재사용
    }

    pass CurtainFillAnim // pass 9 : 커튼 RT 스프라이트애님 별, 알파 기록
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_CurtainOver, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_SPRITEANIM())); // 기존 PS 재사용
    }

    pass AlphaErase // pass 10 : 지우개 (별 모양만큼 RT 알파를 0으로)
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaErase, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_ALPHAERASE()));
    }

    pass CurtainFillAdd // pass 11 : 풀스크린 가산 플래시 (백버퍼 직접)
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_CURTAINFILL())); // PS 그대로 재사용
    }
}