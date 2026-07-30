#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnknownTexture;
Texture2D g_MRATexture;

float4x4 g_BoneMatrices[512];

float2 g_vMaskValue;
float g_fHullThickness;

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);

uint g_iMaterialID = 0;

float4 g_vConstantDiffuse = float4(1.f, 1.f, 1.f, 1.f);
float3 g_vConstantMRA = float3(0.f, 1.f, 1.f);
float4 g_vConstantEmissive = float4(0.f, 0.f, 0.f, 1.f);

float g_fHitFlash = 0.f;
float3 g_vHitFlashColor = float3(1.f, 1.f, 1.f);

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
    
struct VS_OUT_HULL
{
    float4 vPosition : SV_POSITION;
};

/* 정점셰이더 : 정점 데이터의 변환 과정을 수행한다. */

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;    
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    
    float4x4 m0 = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x;
    float4x4 m1 = g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y;
    float4x4 m2 = g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z;
    float4x4 m3 = g_BoneMatrices[In.vBlendIndex.w] * fWeightW;

    float4x4 BoneMatrix = m0 + m1 + m2 + m3;

    
    /* 월드변환, 뷰 벼환, 투영변환 */ 
    float4 vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    float4 vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);
    
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

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

VS_OUT VS_TEST(VS_IN In)
{
    VS_OUT Out;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    
    float4x4 m0 = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x;
    float4x4 m1 = g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y;
    float4x4 m2 = g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z;
    float4x4 m3 = g_BoneMatrices[In.vBlendIndex.w] * fWeightW;

    float4x4 BoneMatrix = m0 + m1 + m2 + m3;

    
    /* 월드변환, 뷰 벼환, 투영변환 */ 
    float4 vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    float4 vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    //float4 vPosition = float4(In.vPosition.rgb, 1.f);
    //float4 vNormal = float4(In.vNormal.rgb, 0.f);

    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vTexcoord1 = In.vTexcoord1;
    Out.vTexcoord2 = In.vTexcoord2;
    Out.vTexcoord3 = In.vTexcoord3;
    Out.vWorldPos = mul(vPosition, g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
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
    uint   vMaterialID : SV_TARGET6;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    
    float3 mra = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord1).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);

    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
    Out.vEmissive.rgb += g_vHitFlashColor * g_fHitFlash;
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    
    return Out;
}

PS_OUT PS_CONSTANT_MATERIAL(PS_IN In)
{
    PS_OUT Out;

    Out.vDiffuse = g_vConstantDiffuse;
    Out.vNormal = float4(normalize(In.vNormal).xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(g_vConstantMRA, 1.f);
    Out.vEmissive = g_vConstantEmissive;
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;

    return Out;
}

PS_OUT PS_OPAQUE(PS_IN In)
{
    PS_OUT Out;
    
    vector vMtrlDiffuse =  g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);

    float3 mra = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);

    float3x3 TBN = float3x3(T, B, N);

    float2 nrg =
          g_NormalTexture.Sample(LinearSampler, In.vTexcoord1).rg;
    float3 nTS =
          float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));

    float3 Nw = mul(nTS, TBN);

    Out.vDiffuse = float4(vMtrlDiffuse.rgb, 1.f);
    Out.vNormal = vector(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
    Out.vEmissive.rgb += g_vHitFlashColor * g_fHitFlash;
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    
    return Out;
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
    pass NonEyePass // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass ConstantMaterialPass // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CONSTANT_MATERIAL();
    }
    pass OpaquePass // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OPAQUE();
    }
}