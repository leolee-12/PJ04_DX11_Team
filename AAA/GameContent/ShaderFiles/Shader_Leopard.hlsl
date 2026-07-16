#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_BoneMatrices[512];

Texture2D g_DiffuseTexture;
Texture2D g_DiffuseTexture1;
Texture2D g_MRATexture;
Texture2D g_MRATexture1;
Texture2D g_NormalTexture;
Texture2D g_NormalTexture1;

Texture2D g_BodyMaskTexture;

float4 g_vEmissiveColor;
uint g_iMaterialID = 0;

float g_fShadowDepthBias = 0.001f;

float g_fHitFlash = 0.f;
float3 g_vHitFlashColor = float3(1.f, 1.f, 1.f);

// 눈 팔레트 (마스크 R=흰자, G=테두리, B=파란 눈동자)
static const float3 EYE_WHITE = float3(1.f, 1.f, 1.f);
static const float3 EYE_BLUE = float3(0.12f, 0.45f, 1.f);
static const float3 EYE_RIM = float3(0.1f, 0.1f, 0.1f);

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;

    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;

    float4 vWorldPos : TEXCOORD4;
    float4 vProjPos : TEXCOORD5;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

float4x4 Compute_BoneMatrix(VS_IN In)
{
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    return g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x
         + g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y
         + g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z
         + g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
}

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4x4 BoneMatrix = Compute_BoneMatrix(In);
    float4 vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    float4 vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);

    float4x4 matWVP = mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix);

    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vTexcoord1 = In.vTexcoord1;
    Out.vWorldPos = mul(vPosition, g_WorldMatrix);
    Out.vProjPos = Out.vPosition;

    float4 vTangent = mul(float4(In.vTangent.xyz, 0.f), BoneMatrix);
    float4 vBinormal = mul(float4(In.vBinormal.xyz, 0.f), BoneMatrix);
    Out.vTangent = normalize(mul(vTangent, g_WorldMatrix));
    Out.vTangent.w = In.vTangent.w;
    Out.vBinormal = normalize(mul(vBinormal, g_WorldMatrix));
    Out.vBinormal.w = In.vBinormal.w;

    return Out;
}

//======== Shadow (depth-only, skinned) ========
struct VS_SHADOW_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
    VS_SHADOW_OUT Out;
    float4 vPos = mul(float4(In.vPosition, 1.f), Compute_BoneMatrix(In));
    float4 vWorld = mul(vPos, g_WorldMatrix);
    Out.vPosition = mul(mul(vWorld, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;
    return Out;
}

struct PS_SHADOW_OUT
{
    float4 vLightDepth : SV_TARGET0;
};

PS_SHADOW_OUT PS_SHADOW(VS_SHADOW_OUT In)
{
    PS_SHADOW_OUT Out;
    float d = In.vProjPos.z / In.vProjPos.w;
    d += g_fShadowDepthBias; // 수신자 기준 뒤로 살짝 밀어 self-shadow 아크네 방지
    Out.vLightDepth = float4(d, 1.f, 0.f, 1.f);
    return Out;
}

//======== G-Buffer 공통 ========
struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vMRA : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
    float4 vGeoNormal : SV_TARGET5;
    uint vMaterialID : SV_TARGET6;
};

// 2채널 노멀맵을 TBN으로 월드 노멀 복원
float3 Sample_WorldNormal(Texture2D tNormal, float2 vUV, VS_OUT In)
{
    float3x3 TBN = float3x3(normalize(In.vTangent.xyz),
                            normalize(In.vBinormal.xyz),
                            normalize(In.vNormal.xyz));
    float2 nrg = tNormal.Sample(LinearSampler, vUV).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    return mul(nTS, TBN);
}

// 디퓨즈 + MRA + 노멀 한 세트 샘플링
void Sample_PBRSet(Texture2D tDiffuse, Texture2D tMRA, Texture2D tNormal, float2 vUV, VS_OUT In,
                   out float3 vAlbedo, out float3 vMRA, out float3 Nw)
{
    vAlbedo = tDiffuse.Sample(LinearSampler, vUV).rgb;
    vMRA = tMRA.Sample(LinearSampler, vUV).rgb;
    Nw = Sample_WorldNormal(tNormal, vUV, In);
}

PS_OUT Make_GBuffer(VS_OUT In, float3 vAlbedo, float3 Nw, float3 vMRA)
{
    PS_OUT Out;
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb + g_vHitFlashColor * g_fHitFlash, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

//======== 픽셀 셰이더 ========
PS_OUT PS_BODY(VS_OUT In)
{
    float3 vAlbedo, vMRA, Nw;
    Sample_PBRSet(g_DiffuseTexture, g_MRATexture, g_NormalTexture, In.vTexcoord, In, vAlbedo, vMRA, Nw);
    
    return Make_GBuffer(In, vAlbedo, Nw, vMRA);
}

PS_OUT PS_EYE(VS_OUT In)
{
    float4 vEyeMask = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    float3 vEyeColor = vEyeMask.r * EYE_WHITE
                     + vEyeMask.g * EYE_RIM
                     + vEyeMask.b * EYE_BLUE;
    float3 vAlbedo = lerp(float3(1.f, 1.f, 1.f), vEyeColor, vEyeMask.a);

    float3 Nw = Sample_WorldNormal(g_NormalTexture, In.vTexcoord, In);
    return Make_GBuffer(In, vAlbedo, Nw, float3(0.f, 1.f, 1.f)); // 눈: 비금속, 러프니스 1
}

technique11 DefaultTechnique
{
    pass ShadowPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
    pass Body // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BODY();
    }
    pass Eye // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EYE();
    }
}