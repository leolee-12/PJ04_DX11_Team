#include "Shader_World_Common.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_WorldMatrixInverse;

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
};

PS_IN VS_MAIN(VS_IN In)
{
    PS_IN Out = (PS_IN) 0;

    float4 vWorldPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vPosition = mul(mul(vWorldPosition, g_ViewMatrix), g_ProjMatrix);
    Out.vNormal = float4(normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix).xyz), 0.f);
    Out.vTexcoord0 = In.vTexcoord0;
    Out.vTexcoord1 = In.vTexcoord1;
    Out.vTexcoord2 = In.vTexcoord2;
    Out.vTexcoord3 = In.vTexcoord3;
    Out.vProjPos = Out.vPosition;
    Out.vTangent = float4(normalize(mul(float4(In.vTangent.xyz, 0.f), g_WorldMatrix).xyz), In.vTangent.w);
    Out.vBinormal = float4(normalize(mul(float4(In.vBinormal.xyz, 0.f), g_WorldMatrix).xyz), In.vBinormal.w);

    return Out;
}



struct VS_SHADOW_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
    float2 vTexcoord : TEXCOORD1;
    float2 vUnknownTexcoord : TEXCOORD2;
};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
    VS_SHADOW_OUT Out = (VS_SHADOW_OUT) 0;

    float4 vWorldPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vPosition = mul(mul(vWorldPosition, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;
    Out.vTexcoord = Apply_UVTransform(Select_UV(In.vTexcoord0, In.vTexcoord1, In.vTexcoord2, In.vTexcoord3, g_iUVIndex), g_vUVTransform);
    Out.vUnknownTexcoord = Apply_UVTransform(Select_UV(In.vTexcoord0, In.vTexcoord1, In.vTexcoord2, In.vTexcoord3, g_iUnknownUVIndex),
      g_vUVTransformUnknown);

    return Out;
}

PS_SHADOW_OUT PS_SHADOW(VS_SHADOW_OUT In)
{
    Apply_ShadowDitherIfNeeded(In.vPosition);
    Apply_ShadowAlphaCut(In.vTexcoord, In.vUnknownTexcoord);
    return Make_ShadowOutput(In.vProjPos);
}



struct VS_DECAL_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

VS_DECAL_OUT VS_DECAL(VS_IN In)
{
    VS_DECAL_OUT Out = (VS_DECAL_OUT) 0;

    float4 vWorldPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vPosition = mul(mul(vWorldPosition, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;

    return Out;
}

PS_DECAL_OUT PS_DECAL(VS_DECAL_OUT In)
{
    Apply_DitherIfNeeded(In.vPosition);

    float2 vScreenUV = Get_ScreenUV(In.vProjPos);
    float fSceneDepth = Sample_SceneDepth(vScreenUV);
    float3 vWorldPosition = Recover_SceneWorldPosition(vScreenUV, fSceneDepth);
    float3 vLocalPosition = mul(float4(vWorldPosition, 1.f), g_WorldMatrixInverse).xyz;

    Apply_DecalBoundsCut(vLocalPosition);
    Apply_DecalMaterialMask(In.vPosition.xy);

    float2 vDecalUV = Get_DecalUV(vLocalPosition);
    return Make_DecalOutput(vDecalUV, g_WorldMatrix._11_12_13, g_WorldMatrix._21_22_23, -g_WorldMatrix._31_32_33);
}



#include "Shader_World_Technique.hlsli"