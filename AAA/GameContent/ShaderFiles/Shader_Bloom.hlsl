#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_SceneTexture; // HDR 씬
Texture2D g_BloomTexture; // 블러 대상
float2 g_vBlurDir; // (1,0)=H, (0,1)=V
float2 g_vTexelSize; // 1/해상도 (블룸 타겟 기준)
float g_fThreshold = 1.0f;
float g_fBloomIntensity = 1.0f;

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
struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    float4 p = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    p = mul(p, g_ViewMatrix);
    p = mul(p, g_ProjMatrix);
    Out.vPosition = p;
    Out.vTexcoord = In.vTexcoord;
    return Out;
}

  // pass 0 : 밝은 부분 추출
float4 PS_BRIGHT(PS_IN In) : SV_TARGET0
{
    float3 c = g_SceneTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float luma = dot(c, float3(0.2126f, 0.7152f, 0.0722f));
    float k = max(luma - g_fThreshold, 0.f) / max(luma, 1e-4f); // soft knee
    return float4(c * k, 1.f);
}

  // pass 1 : 분리형 가우시안 (H/V는 g_vBlurDir로)
float4 PS_BLUR(PS_IN In) : SV_TARGET0
{
    const float w[5] = { 0.227027f, 0.194594f, 0.121622f, 0.054054f, 0.016216f };
    float3 result = g_BloomTexture.Sample(LinearSampler, In.vTexcoord).rgb * w[0];
      [unroll]
    for (int i = 1; i < 5; ++i)
    {
        float2 off = g_vBlurDir * g_vTexelSize * i;
        result += g_BloomTexture.Sample(LinearSampler, In.vTexcoord + off).rgb * w[i];
        result += g_BloomTexture.Sample(LinearSampler, In.vTexcoord - off).rgb * w[i];
    }
    return float4(result, 1.f);
}

  // pass 2 : 합성 + 톤매핑 → 백버퍼
float4 PS_COMPOSITE(PS_IN In) : SV_TARGET0
{
    float3 scene = g_SceneTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float3 bloom = g_BloomTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float3 color = scene + bloom * g_fBloomIntensity;
    color = color / (color + 1.f); // Reinhard
    color = pow(color, 1.f / 2.2f); // 감마
    return float4(color, 1.f);
}

technique11 DefaultTechnique
{
    pass Bright
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_BRIGHT();
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
    }
    pass Blur
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_BLUR();
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
    }
    pass Composite
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_COMPOSITE();
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
    }
}