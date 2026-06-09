#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

bool g_bAlphaTest = false;
int g_iInvertMask = 0;

int g_iFrame = { 0 }; // SpriteAnim 슬라이스 선택
int g_iMaskChannel = 3; // 0 : R, 1 : G, 2 : B, 3 : A

float g_fAlpha = { 1.f };
float g_fTestAlpha = { 0.f };
float g_fMaskPower = { 1.f };
float g_fEffectIntensity = { 1.f };

float2 g_vEffectTiling = float2(1.f, 1.f);
float2 g_vEffectOffset = float2(0.f, 0.f);

float4 g_vColor = { 1.f, 1.f, 1.f, 1.f };

Texture2D g_Texture;
Texture2D g_EffectTexture;
Texture2D g_MaskTexture;

Texture2DArray g_TextureArray;              // SpriteAnim용

// 마스크 채널 선택 helper
float SampleMask(float2 uv)
{
    float4 maskTex = g_MaskTexture.Sample(UISampler, uv);
    
    float mask = maskTex.a;

    if (g_iMaskChannel == 0)
        mask = maskTex.r;
    else if (g_iMaskChannel == 1)
        mask = maskTex.g;
    else if (g_iMaskChannel == 2)
        mask = maskTex.b;
   
    if (g_iInvertMask != 0)
        mask = 1.f - mask;
    
    mask = saturate(mask);
    mask = pow(mask, max(g_fMaskPower, 0.0001f));
    
    return mask;    
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
    Out.vColor = g_Texture.Sample(UISampler, In.vTexcoord);
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

PS_OUT PS_MASKED_TEXTURE(PS_IN In)
{
    PS_OUT Out;
    
    float mask = SampleMask(In.vTexcoord);
    
    float2 effectUV = In.vTexcoord * g_vEffectTiling + g_vEffectOffset;
    float4 effect = g_EffectTexture.Sample(LinearSampler, effectUV);
    
    effect.rgb *= g_vColor.rgb * g_fEffectIntensity;
    effect.a *= g_vColor.a * g_fAlpha * mask;
    
    if (effect.a <= 0.f)
        discard;
    
    Out.vColor = effect;
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
}