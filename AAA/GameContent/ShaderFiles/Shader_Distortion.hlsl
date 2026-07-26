#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_Texture; // 노말맵 (rg = 탄젠트공간 XY, 나머지 채널 미사용)
bool g_bRadialFromUV = { false }; // 노말맵 대신 UV 방사방향 (폴백)
bool g_bViewSpaceNormal = { true }; // true=TBN으로 뷰공간 변환 / false=빌보드(탄젠트~화면)
bool g_bFlipGreen = { false }; // 노말맵 G 규약(OpenGL식이면 true)
float g_fStrength = { 0.03f };
float g_fAlpha = { 1.f };
float2 g_vTiling = { 1.f, 1.f };
float2 g_vOffset = { 0.f, 0.f };
bool g_bUseUVEdgeFade = { false };
int g_iUVEdgeFadeAxis = { 0 };
float g_fUVEdgeFadeStartRange = { 0.1f };
float g_fUVEdgeFadeEndRange = { 0.1f };
float g_fUVEdgeFadePower = { 1.f };
bool g_bLinearReveal = { false };
float g_fLinearRevealRatio = { 1.f };
int g_iLinearRevealAxis = { 0 };
bool g_bLinearRevealReverse = { false };
bool g_bLinearHide = { false };
float g_fLinearHideRatio = { 1.f };
int g_iLinearHideAxis = { 0 };
bool g_bLinearHideReverse = { false };

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
    float3 vNormalV : TEXCOORD1; // 뷰공간 TBN
    float3 vTangentV : TEXCOORD2;
    float3 vBinormalV : TEXCOORD3;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 vWorld = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    float4 vView = mul(vWorld, g_ViewMatrix);
    Out.vPosition = mul(vView, g_ProjMatrix);
    Out.vTexcoord = In.vTexcoord;

    // 월드 -> 뷰 공간 TBN (오프셋을 화면 축에 맞추기 위함)
    float3x3 W = (float3x3) g_WorldMatrix;
    float3x3 V = (float3x3) g_ViewMatrix;
    Out.vNormalV = normalize(mul(mul(In.vNormal, W), V));
    Out.vTangentV = normalize(mul(mul(In.vTangent, W), V));
    Out.vBinormalV = normalize(mul(mul(In.vBinormal, W), V));

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float3 vNormalV : TEXCOORD1;
    float3 vTangentV : TEXCOORD2;
    float3 vBinormalV : TEXCOORD3;
};

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

float ComputeUVEdgeFade(float2 vTexcoord)
{
    if (g_bUseUVEdgeFade == false)
        return 1.f;

    float fFadeX = ComputeUVEdgeFade1D(
        vTexcoord.x, g_fUVEdgeFadeStartRange, g_fUVEdgeFadeEndRange);
    float fFadeY = ComputeUVEdgeFade1D(
        vTexcoord.y, g_fUVEdgeFadeStartRange, g_fUVEdgeFadeEndRange);
    float fFade = g_iUVEdgeFadeAxis == 0
        ? fFadeX : (g_iUVEdgeFadeAxis == 1 ? fFadeY : fFadeX * fFadeY);

    return pow(saturate(fFade), clamp(g_fUVEdgeFadePower, 0.1f, 8.f));
}

float4 PS_WRITE_OFFSET(PS_IN In) : SV_TARGET
{
    if (g_bLinearReveal)
    {
        float revealCoord =
            g_iLinearRevealAxis == 1 ? In.vTexcoord.y : In.vTexcoord.x;
        if (g_bLinearRevealReverse)
            revealCoord = 1.f - revealCoord;
        if (revealCoord > saturate(g_fLinearRevealRatio))
            discard;
    }

    if (g_bLinearHide)
    {
        float hideCoord =
            g_iLinearHideAxis == 1 ? In.vTexcoord.y : In.vTexcoord.x;
        if (g_bLinearHideReverse)
            hideCoord = 1.f - hideCoord;
        if (hideCoord > saturate(g_fLinearHideRatio))
            discard;
    }

    float2 uv = In.vTexcoord * g_vTiling + g_vOffset;

    float2 dir;
    if (g_bRadialFromUV)
    {
        dir = normalize(In.vTexcoord - 0.5f + 1e-5f);
    }
    else
    {
        // 노말맵 rg 자체가 방향 + 세기 (평평하면 0 -> 왜곡 없음)
        float2 nRG = g_Texture.Sample(LinearSampler, uv).rg * 2.f - 1.f;
        if (g_bFlipGreen)
            nRG.y = -nRG.y;

        if (g_bViewSpaceNormal)
        {
            float3 nT = float3(nRG, sqrt(saturate(1.f - dot(nRG, nRG))));
            float3x3 TBN = float3x3(normalize(In.vTangentV),
                                    normalize(In.vBinormalV),
                                    normalize(In.vNormalV));
            float3 nV = normalize(mul(nT, TBN));
            // 뷰공간 노말의 기울기만 취함(정면일수록 0에 수렴)
            dir = nV.xy;
        }
        else
        {
            dir = nRG; // 빌보드: 탄젠트공간 ~ 화면공간
        }
    }

    // 뷰/월드는 +Y가 위, 화면 UV는 +Y가 아래 -> Y 뒤집기
    dir.y = -dir.y;

    float fUVEdgeFade = ComputeUVEdgeFade(In.vTexcoord);
    float2 offset = dir * g_fStrength * g_fAlpha * fUVEdgeFade;

    return float4(offset, 0.f, 1.f); // a=1 : BS_Additive가 SrcAlpha 곱하는 경우 대비
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