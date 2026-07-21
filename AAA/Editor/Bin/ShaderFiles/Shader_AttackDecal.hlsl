#include "Engine_Shader_Defines.hlsli"   // RecoverWorldPos, 샘플러, RS/DSS/BS 상태

/* ---- VS 투영용 ---- */
float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
/* ---- PS 복원용 ---- */
float4x4 g_WorldMatrixInverse, g_ViewMatrixInverse, g_ProjMatrixInverse;

/* ---- 씬 G버퍼 ---- */
Texture2D g_DepthTexture;
Texture2D<uint> g_MaterialIDTexture;
/* ---- 데칼 ---- */
Texture2D g_DiffuseTexture;
float3 g_vDecalBoundsCenter = float3(0.f, 0.f, 0.f);
float3 g_vDecalBoundsExtents = float3(0.5f, 0.5f, 0.5f);
float g_fDecalAlpha = 1.f;
int g_iDecalMaskMode = 1; // 1 = "이 ID에만" (정적 바닥)
int g_iDecalMaskID = 1; // WORLD_STATIC_ID

struct VS_IN
{
    float3 vPosition : POSITION;
};
struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vMRA : SV_TARGET2;
};

VS_OUT VS_DECAL(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    float4 wp = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vPosition = mul(mul(wp, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;
    return Out;
}

PS_OUT PS_DECAL(VS_OUT In)
{
    float2 suv = float2(In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f,
                        In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f);

    float depth = g_DepthTexture.Sample(PointSampler, suv).x;
    if (depth <= 0.f)
        discard;

    float3 wp = RecoverWorldPos(suv, depth, g_ProjMatrixInverse, g_ViewMatrixInverse);
    float3 lp = mul(float4(wp, 1.f), g_WorldMatrixInverse).xyz;

    if (any(abs(lp - g_vDecalBoundsCenter) > g_vDecalBoundsExtents + 1e-4f))
        discard;

    uint id = g_MaterialIDTexture.Load(int3(In.vPosition.xy, 0));
    if ((0 == g_iDecalMaskMode && id == (uint) g_iDecalMaskID) ||
        (0 != g_iDecalMaskMode && id != (uint) g_iDecalMaskID))
        discard;

    float2 uv;
    uv.x = (lp.x - g_vDecalBoundsCenter.x) / (2.f * g_vDecalBoundsExtents.x) + 0.5f;
    uv.y = 0.5f - (lp.z - g_vDecalBoundsCenter.z) / (2.f * g_vDecalBoundsExtents.z);

    float4 col = g_DiffuseTexture.Sample(LinearSampler, uv);
    if (col.a <= 0.f)
        discard;
    float coverage = col.a * g_fDecalAlpha;

    PS_OUT Out = (PS_OUT) 0;
    Out.vDiffuse = float4(col.rgb, coverage);
    Out.vNormal = float4(0.f, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 0.f, 0.f, 0.f);
    return Out;
}

technique11 DefaultTechnique
{
    pass Decal // pass 0
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Decal, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DECAL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DECAL();
    }
}