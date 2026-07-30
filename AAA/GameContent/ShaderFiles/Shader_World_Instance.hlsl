#include "Shader_World_Common.hlsli"

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord0 : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float4 vDissolveParams : DISSOLVE;

    row_major float4x4 WorldMatrix : WORLD;
};

PS_IN VS_MAIN(VS_IN In)
{
    PS_IN Out = (PS_IN) 0;

    float4 vWorldPosition = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
    Out.vPosition = mul(mul(vWorldPosition, g_ViewMatrix), g_ProjMatrix);
    Out.vNormal = float4(normalize(mul(float4(In.vNormal, 0.f), In.WorldMatrix).xyz), 0.f);
    Out.vTexcoord0 = In.vTexcoord0;
    Out.vTexcoord1 = In.vTexcoord1;
    Out.vTexcoord2 = In.vTexcoord2;
    Out.vTexcoord3 = In.vTexcoord3;
    Out.vProjPos = Out.vPosition;
    Out.vDissolveParams = float2(In.vDissolveParams.x, In.vDissolveParams.z);
    Out.vTangent = float4(normalize(mul(float4(In.vTangent.xyz, 0.f), In.WorldMatrix).xyz), In.vTangent.w);
    Out.vBinormal = float4(normalize(mul(float4(In.vBinormal.xyz, 0.f), In.WorldMatrix).xyz), In.vBinormal.w);

    return Out;
}



struct VS_SHADOW_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
    float2 vTexcoord : TEXCOORD1;
    float2 vUnknownTexcoord : TEXCOORD2;
    nointerpolation float fDissolve : TEXCOORD3;
};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
    VS_SHADOW_OUT Out = (VS_SHADOW_OUT) 0;

    float4 vWorldPosition = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
    Out.vPosition = mul(mul(vWorldPosition, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;
    Out.fDissolve = In.vDissolveParams.y;
    Out.vTexcoord = Apply_UVTransform(Select_UV(In.vTexcoord0, In.vTexcoord1, In.vTexcoord2, In.vTexcoord3, g_iUVIndex), g_vUVTransform);
    Out.vUnknownTexcoord = Apply_UVTransform(Select_UV(In.vTexcoord0, In.vTexcoord1, In.vTexcoord2, In.vTexcoord3, g_iUnknownUVIndex),
      g_vUVTransformUnknown);

    return Out;
}

PS_SHADOW_OUT PS_SHADOW(VS_SHADOW_OUT In)
{
    Apply_ShadowDither(In.vPosition, In.fDissolve);
    Apply_ShadowAlphaCut(In.vTexcoord, In.vUnknownTexcoord);
    return Make_ShadowOutput(In.vProjPos);
}



struct VS_DECAL_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;

    nointerpolation float4 vMatrixRow0 : TEXCOORD1;
    nointerpolation float4 vMatrixRow1 : TEXCOORD2;
    nointerpolation float4 vMatrixRow2 : TEXCOORD3;

    nointerpolation float4 vInvRow0 : TEXCOORD4;
    nointerpolation float4 vInvRow1 : TEXCOORD5;
    nointerpolation float4 vInvRow2 : TEXCOORD6;
    nointerpolation float4 vInvRow3 : TEXCOORD7;

    nointerpolation float2 vInstanceParams : TEXCOORD8; // x: Dissolve, y: DecalAlpha
};

float3x3 Build_Inverse3x3(float3 vRow0, float3 vRow1, float3 vRow2)
{
    float3 vC0 = cross(vRow1, vRow2);
    float3 vC1 = cross(vRow2, vRow0);
    float3 vC2 = cross(vRow0, vRow1);
    float fDet = dot(vRow0, vC0);
    float fSafeDet = abs(fDet) < 1e-6f ? (fDet < 0.f ? -1e-6f : 1e-6f) : fDet;
    float fInvDet = rcp(fSafeDet);

    return float3x3(vC0.x, vC1.x, vC2.x, vC0.y, vC1.y, vC2.y, vC0.z, vC1.z, vC2.z) * fInvDet;
}

VS_DECAL_OUT VS_DECAL(VS_IN In)
{
    VS_DECAL_OUT Out = (VS_DECAL_OUT) 0;

    float4 vWorldPosition = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
    Out.vPosition = mul(mul(vWorldPosition, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;

    float3 vRow0 = In.WorldMatrix[0].xyz;
    float3 vRow1 = In.WorldMatrix[1].xyz;
    float3 vRow2 = In.WorldMatrix[2].xyz;
    float3 vTrans = In.WorldMatrix[3].xyz;

    float3x3 matInv3 = Build_Inverse3x3(vRow0, vRow1, vRow2);
    float3 vInvTrans = mul(-vTrans, matInv3);

    Out.vMatrixRow0 = In.WorldMatrix[0];
    Out.vMatrixRow1 = In.WorldMatrix[1];
    Out.vMatrixRow2 = In.WorldMatrix[2];
    Out.vInvRow0 = float4(matInv3[0], 0.f);
    Out.vInvRow1 = float4(matInv3[1], 0.f);
    Out.vInvRow2 = float4(matInv3[2], 0.f);
    Out.vInvRow3 = float4(vInvTrans, 1.f);
    Out.vInstanceParams = In.vDissolveParams.xw;
    
    return Out;
}

PS_DECAL_OUT PS_DECAL(VS_DECAL_OUT In)
{
    Apply_Dither(In.vPosition, In.vInstanceParams.x);

    float2 vScreenUV = Get_ScreenUV(In.vProjPos);
    float fSceneDepth = Sample_SceneDepth(vScreenUV);
    float3 vWorldPosition = Recover_SceneWorldPosition(vScreenUV, fSceneDepth);

    row_major float4x4 matInv = float4x4(In.vInvRow0, In.vInvRow1, In.vInvRow2, In.vInvRow3);
    float3 vLocalPosition = mul(float4(vWorldPosition, 1.f), matInv).xyz;

    Apply_DecalBoundsCut(vLocalPosition);
    Apply_DecalMaterialMask(In.vPosition.xy);

    float2 vDecalUV = Get_DecalUV(vLocalPosition);
    return Make_DecalOutput(vDecalUV, In.vMatrixRow0.xyz, In.vMatrixRow1.xyz, -In.vMatrixRow2.xyz, In.vInstanceParams.y);
}



// 인스턴스 데칼: depth test + bias(RS_Decal)로 오버드로 절감. 카메라가 데칼 볼륨 안으로 들어가지 않는 용도 전제.
#define WORLD_DECAL_RS  RS_Decal
#define WORLD_DECAL_DSS DSS_NoWrite
#include "Shader_World_Technique.hlsli"