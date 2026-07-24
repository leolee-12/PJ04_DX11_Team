#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_Texture; // 왜곡 소스: rg=방향(노멀맵식), a=프로파일(링 세기)
bool g_bRadialFromUV = { false }; // true면 텍스처 대신 중심기준 UV 방사방향 사용
float g_fStrength = { 0.03f }; // 최대 화면 오프셋(UV 비율)
float g_fAlpha = { 1.f }; // 링 수명 페이드
float2 g_vTiling = { 1.f, 1.f };
float2 g_vOffset = { 0.f, 0.f };

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
    float4 vWorld = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    float4 vView = mul(vWorld, g_ViewMatrix);
    Out.vPosition = mul(vView, g_ProjMatrix);
    Out.vTexcoord = In.vTexcoord;
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

// 링을 그리며 화면 오프셋 벡터만 RG로 출력 (가산블렌드로 Target_Distortion에 누적)
float4 PS_WRITE_OFFSET(PS_IN In) : SV_TARGET
{
    float2 uv = In.vTexcoord * g_vTiling + g_vOffset;
    float4 d = g_Texture.Sample(LinearSampler, uv);

    float2 dir = g_bRadialFromUV
        ? normalize(In.vTexcoord - 0.5f + 1e-5f)
        : (d.rg * 2.f - 1.f);

    float profile = d.a;
    float2 offset = dir * profile * g_fStrength * g_fAlpha;

    return float4(offset, 0.f, 1.f);
}

technique11 DefaultTechnique
{
    pass WriteOffset // 0 : 깊이테스트(DSV 물렸을 때), 깊이쓰기X, 가산
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_WRITE_OFFSET()));
    }

    pass WriteOffset_NoDepth // 1 : 깊이 무시
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_WRITE_OFFSET()));
    }
}