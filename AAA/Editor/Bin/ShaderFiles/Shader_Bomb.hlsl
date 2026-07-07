#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_BoneMatrices[512];

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnknownTexture;
Texture2D g_BodyMaskTexture;
Texture2D g_FuseMaskTexture;
Texture2D g_FuseBurntTexture;
float   g_fBurnRatio;
float3   g_vGlow;

Texture2D g_MRATexture;

float4 g_vEmissiveColor;
uint g_iMaterialID = 0;

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
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;
    
    float4 vWorldPos : TEXCOORD4;
    float4 vProjPos : TEXCOORD5;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    float4x4 BoneMatrix =
            g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x
          + g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y
          + g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z
          + g_BoneMatrices[In.vBlendIndex.w] * fWeightW;

    float4 vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    float4 vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);

    float4x4 matWV = mul(g_WorldMatrix, g_ViewMatrix);
    float4x4 matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vTexcoord1 = In.vTexcoord1;
    Out.vTexcoord2 = In.vTexcoord2;
    Out.vTexcoord3 = In.vTexcoord3;
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
    float fW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    float4x4 Bone = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x
                  + g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y
                  + g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z
                  + g_BoneMatrices[In.vBlendIndex.w] * fW;
    float4 vPos = mul(float4(In.vPosition, 1.f), Bone);
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
    Out.vLightDepth = float4(d, 1.f, 0.f, 1.f);
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;
    
    float4 vWorldPos : TEXCOORD4;
    float4 vProjPos : TEXCOORD5;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vMRA : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
    float4 vGeoNormal : SV_TARGET5;
};

PS_OUT PS_BODY(PS_IN In)
{
    PS_OUT Out;

    float3 vAlbedo = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 vMRA = g_MRATexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);
    
   
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, g_iMaterialID / 255.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb + g_vGlow, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);

    return Out;
}

PS_OUT PS_FUSE(PS_IN In)
{
    PS_OUT Out;

    //float fBurn = 1.f - g_FuseBurntTexture.Sample(ClampSampler, In.vTexcoord).r;
    float fBurn = In.vTexcoord.y;
    
    clip(fBurn - g_fBurnRatio); // Åº ºÎºÐ Á¦°Å
    
    float3 vAlbedo = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float fChar = smoothstep(g_fBurnRatio, g_fBurnRatio + 0.15f, fBurn);
    vAlbedo *= lerp(0.2f, 1.f, fChar);
    
    float3 vMRA = g_MRATexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);
   
    float fEmber = 1.f - smoothstep(g_fBurnRatio, g_fBurnRatio + 0.05f, fBurn);
    
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
    
    Out.vEmissive = float4(g_vEmissiveColor.rgb + float3(1.f, 0.35f, 0.05f) * fEmber, 1.f);
    //Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);

    return Out;
}

technique11 DefaultTechnique
{
    pass Body // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BODY();
    }
    pass Fuse // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FUSE();
    }
    pass ShadowPass // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
}