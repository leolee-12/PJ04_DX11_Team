#include "Engine_Shader_Defines.hlsli"

float4x4 g_ViewMatrix, g_ProjMatrix;
float4x4 g_ViewMatrixInverse, g_ProjMatrixInverse;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnknownTexture;
Texture2D g_MRATexture;

float2 g_vMaskValue;
float4 g_vBlendColor;

float g_NormalStrength = 1.f;
float g_MaskStrength = 1.f;
float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);

uint g_iMaterialID = 0;

Texture2D g_DepthTexture;
Texture2D<uint> g_MaterialIDTexture;
float3 g_vDecalBoundsCenter;
float3 g_vDecalBoundsExtents;
float g_fDecalAlpha = 1.f;
float g_fDecalHasNormal = 0.f;
float g_fDecalHasMRA = 0.f;
int g_iDecalMaskMode = 0;
int g_iDecalMaskID = 200;

float4 g_vColor = float4(1.f, 1.f, 1.f, 1.f);
float3 g_vMRA = float3(0.f, 1.f, 1.f);

// 0=TEXCOORD0, 1=TEXCOORD1, 2=TEXCOORD2, 3=TEXCOORD3
uint g_iUVIndex = 0;
float4 g_vUVTransform = float4(1.f, 1.f, 0.f, 0.f);
uint g_iUnknownUVIndex = 0;
float4 g_vUVTransformNormal = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformMaterial = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformUnknown = float4(1.f, 1.f, 0.f, 0.f);
float g_fUVRotate = 0.f;

#define ENV_INSTANCE_FLAG_DITHER 0x01
uint g_iEnvInstanceFlags = 0;

float g_fDissolve;

#define ENV_SHADOW_ALPHA_NONE 0u
#define ENV_SHADOW_ALPHA_DIFFUSE 1u
#define ENV_SHADOW_ALPHA_UNKNOWN 2u
#define ENV_SHADOW_ALPHA_DISCARD_ALL 3u
#define ENV_SHADOW_ALPHA_DIFFUSE_R 4u
#define ENV_SHADOW_ALPHA_UNKNOWN_R 5u
uint g_iShadowAlphaSource = ENV_SHADOW_ALPHA_NONE;

static const float Bayer4x4[16] =
{
	0.0 / 16.0, 8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0,
	12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0, 6.0 / 16.0,
	3.0 / 16.0, 11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0,
	15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0, 5.0 / 16.0
};

void Apply_Dissolve(float4 vScreenPos)
{
	float fVisibility = 1.f - g_fDissolve;
	int2 px = int2(vScreenPos.xy) & 3;
	if (fVisibility <= Bayer4x4[px.y * 4 + px.x])
		discard;
}

void Apply_Dither_IfNeeded(float4 vScreenPos)
{
	[branch]
	if (0 != (g_iEnvInstanceFlags & ENV_INSTANCE_FLAG_DITHER))
		Apply_Dissolve(vScreenPos);
}

float2 ApplyMeshUVTransform(float2 uv)
{
	return uv * g_vUVTransform.xy + g_vUVTransform.zw;
}

float2 ApplyUnknownUVTransform(float2 uv)
{
    return uv * g_vUVTransformUnknown.xy + g_vUVTransformUnknown.zw;
}

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

	row_major float4x4 WorldMatrix : WORLD;
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

float2 Select_MeshUV_VS(VS_IN In, uint iUVIndex)
{
      [branch] if (1u == iUVIndex) return In.vTexcoord1;
      [branch] if (2u == iUVIndex) return In.vTexcoord2;
      [branch] if (3u == iUVIndex) return In.vTexcoord3;
      return In.vTexcoord;
}

float2 Get_BaseUV_VS(VS_IN In)
{
      return ApplyMeshUVTransform(Select_MeshUV_VS(In, g_iUVIndex));
}

float2 Get_UnknownUV_VS(VS_IN In)
{
      return ApplyUnknownUVTransform(Select_MeshUV_VS(In, g_iUnknownUVIndex));
}

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out;

	float4 vPosition = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
	vPosition = mul(vPosition, g_ViewMatrix);
	vPosition = mul(vPosition, g_ProjMatrix);

	Out.vPosition = vPosition;
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), In.WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vTexcoord1 = In.vTexcoord1;
    Out.vTexcoord2 = In.vTexcoord2;
    Out.vTexcoord3 = In.vTexcoord3;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
	Out.vProjPos = Out.vPosition;

	float3 T = normalize(In.vTangent.xyz);
	float3 B = normalize(In.vBinormal.xyz);
	Out.vTangent = normalize(mul(float4(T, 0.f), In.WorldMatrix));
	Out.vTangent.w = In.vTangent.w;
	Out.vBinormal = normalize(mul(float4(B, 0.f), In.WorldMatrix));
	Out.vBinormal.w = In.vBinormal.w;

	return Out;
}

//============================ Shadow (depth-only) ============================
struct VS_SHADOW_OUT
{
	float4 vPosition : SV_POSITION;
	float4 vProjPos : TEXCOORD0;
	float2 vTexcoord : TEXCOORD1;
    float2 vUnknownTexcoord : TEXCOORD2;

};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
	VS_SHADOW_OUT Out;

	float4 vWorld = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
	Out.vPosition = mul(mul(vWorld, g_ViewMatrix), g_ProjMatrix);
	Out.vProjPos = Out.vPosition;
    Out.vTexcoord = Get_BaseUV_VS(In);
    Out.vUnknownTexcoord = Get_UnknownUV_VS(In);
	return Out;
}

struct PS_SHADOW_OUT
{
	float4 vLightDepth : SV_TARGET0;
};

void Apply_ShadowAlphaCut(float2 vUV, float2 vUnknownUV)
{
	[branch]
	if (ENV_SHADOW_ALPHA_DISCARD_ALL == g_iShadowAlphaSource)
		discard;

	float fAlpha = 1.f;

	[branch]
	if (ENV_SHADOW_ALPHA_DIFFUSE == g_iShadowAlphaSource)
		fAlpha = g_DiffuseTexture.Sample(LinearSampler, vUV).a;
	else if (ENV_SHADOW_ALPHA_UNKNOWN == g_iShadowAlphaSource)
		fAlpha = g_UnknownTexture.Sample(LinearSampler, vUnknownUV).a;
	else if (ENV_SHADOW_ALPHA_DIFFUSE_R == g_iShadowAlphaSource)
		fAlpha = g_DiffuseTexture.Sample(LinearSampler, vUV).r;
    else if (ENV_SHADOW_ALPHA_UNKNOWN_R == g_iShadowAlphaSource)
        fAlpha = g_UnknownTexture.Sample(LinearSampler, vUnknownUV).r;
	
	if (fAlpha < 0.1f)
		discard;
}

PS_SHADOW_OUT PS_SHADOW(VS_SHADOW_OUT In)
{
    Apply_ShadowAlphaCut(In.vTexcoord, In.vUnknownTexcoord);

	PS_SHADOW_OUT Out;
	float d = In.vProjPos.z / In.vProjPos.w;
	Out.vLightDepth = float4(d, d, d, 1.f);
	return Out;
}

// Main
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

struct PS_BACKOUT
{
	float4 vDiffuse : SV_TARGET0;
};

float3 PerturbNormal(float3 N, float3 worldPos, float2 uv, float3 nTS)
{
	float3 dp1 = ddx(worldPos), dp2 = ddy(worldPos);
	float2 d1 = ddx(uv), d2 = ddy(uv);
	float3 dp2perp = cross(dp2, N);
	float3 dp1perp = cross(N, dp1);
	float3 T = dp2perp * d1.x + dp1perp * d2.x;
	float3 B = dp2perp * d1.y + dp1perp * d2.y;
	float inv = rsqrt(max(dot(T, T), dot(B, B)));

	// 베이스 노멀은 N 그대로, 접선방향 디테일만 더함
	return normalize(N + (T * inv * nTS.x + B * inv * nTS.y));
}

float2 Select_MeshUV_PS(PS_IN In, uint iUVIndex)
{
      [branch] if (1u == iUVIndex) return In.vTexcoord1;
      [branch] if (2u == iUVIndex) return In.vTexcoord2;
      [branch] if (3u == iUVIndex) return In.vTexcoord3;
      return In.vTexcoord;
}

float2 Get_BaseUV(PS_IN In)
{
    return ApplyMeshUVTransform(Select_MeshUV_PS(In, g_iUVIndex));
}
float2 Get_NormalUV(PS_IN In)
{
    return Select_MeshUV_PS(In, g_iUVIndex) * g_vUVTransformNormal.xy + g_vUVTransformNormal.zw;
}
float2 Get_MaterialUV(PS_IN In)
{
    return Select_MeshUV_PS(In, g_iUVIndex) * g_vUVTransformMaterial.xy + g_vUVTransformMaterial.zw;
}
float2 Get_UnknownUV(PS_IN In)
{
    return ApplyUnknownUVTransform(Select_MeshUV_PS(In, g_iUnknownUVIndex));
}

PS_OUT PS_WHITE(PS_IN In)
{
	PS_OUT Out;

	Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
	Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
	Out.vEmissive = float4(0.f, 0.f, 0.f, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;

	return Out;
}

PS_OUT PS_DIFF_SAMPLE(PS_IN In, float2 vUV)
{
	Apply_Dither_IfNeeded(In.vPosition);

	PS_OUT Out;
	vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, vUV);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
}

PS_OUT PS_DMN_SAMPLE(PS_IN In, float2 vBaseUV, float2 vNormalUV, float2 vMaterialUV)
{
	Apply_Dither_IfNeeded(In.vPosition);

	PS_OUT Out;
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, vBaseUV);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

    float3 vMRA = g_MRATexture.Sample(LinearSampler, vMaterialUV).rgb;

	float3 N = normalize(In.vNormal);
	float3 T = normalize(In.vTangent.xyz);
	float3 B = normalize(In.vBinormal.xyz);

	float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg;
	float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));

	float3 Nw = mul(nTS, TBN);

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(Nw * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
}

PS_OUT PS_UKWN_SAMPLE(PS_IN In, float2 vUV)
{
	Apply_Dither_IfNeeded(In.vPosition);

	PS_OUT Out;
	vector vMtrlDiffuse = g_UnknownTexture.Sample(LinearSampler, vUV);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
}

PS_OUT PS_UMN_SAMPLE(PS_IN In, float2 vUnknownUV, float2 vNormalUV, float2 vMaterialUV)
{
	Apply_Dither_IfNeeded(In.vPosition);

	PS_OUT Out;
    vector vMtrlDiffuse = g_UnknownTexture.Sample(LinearSampler, vUnknownUV);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

    float3 vMRA = g_MRATexture.Sample(LinearSampler, vMaterialUV).rgb;

	float3 N = normalize(In.vNormal);
	float3 T = normalize(In.vTangent.xyz);
	float3 B = normalize(In.vBinormal.xyz);

	float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg;
	float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));

	float3 Nw = mul(nTS, TBN);

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(Nw * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
}

PS_OUT PS_DIFF(PS_IN In)
{
    return PS_DIFF_SAMPLE(In, Get_BaseUV(In));
}

PS_OUT PS_DMN(PS_IN In)
{
    return PS_DMN_SAMPLE(In, Get_BaseUV(In), Get_NormalUV(In), Get_MaterialUV(In));
}

PS_OUT PS_UKWN(PS_IN In)
{
    return PS_UKWN_SAMPLE(In, Get_UnknownUV(In));
}

PS_OUT PS_UMN(PS_IN In)
{
    return PS_UMN_SAMPLE(In, Get_UnknownUV(In), Get_NormalUV(In), Get_MaterialUV(In));
}

PS_OUT PS_DMNU(PS_IN In)
{
    float fMask = g_UnknownTexture.Sample(LinearSampler, Get_UnknownUV(In)).r;
    if (fMask < 0.1f)
        discard;

	PS_OUT Out = PS_DMN_SAMPLE(In, Get_BaseUV(In), Get_NormalUV(In), Get_MaterialUV(In));

    return Out;
}

PS_OUT PS_TREESHADOW(PS_IN In)
{
    float fMask = g_DiffuseTexture.Sample(LinearSampler, Get_UnknownUV(In)).r;
    if (fMask < 0.1f)
        discard;

    PS_OUT Out = PS_DIFF_SAMPLE(In, Get_BaseUV(In));
    return Out;
}

PS_OUT PS_GRASS_FUR(PS_IN In)
{
    PS_OUT Out = PS_DIFF_SAMPLE(In, Get_BaseUV(In));

	return Out;
}

PS_OUT PS_MN(PS_IN In)
{
	Apply_Dither_IfNeeded(In.vPosition);

	PS_OUT Out;

	float3 vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

	float3 N = normalize(In.vNormal);
	float3 T = normalize(In.vTangent.xyz);
	float3 B = normalize(In.vBinormal.xyz);

	float3x3 TBN = float3x3(T, B, N);

	float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
	float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));

	float3 Nw = mul(nTS, TBN);

	Out.vDiffuse = vector(0.294f, 0.424f, 0.235f, 1.f);
	Out.vNormal = vector(Nw * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
	Out.vEmissive = vector(0.f, 0.f, 0.f, 0.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
}

PS_OUT PS_DISCARD(PS_IN In)
{
	discard;
	return (PS_OUT)0;
}

PS_OUT PS_COLOR_MRA_DITHER(PS_IN In)
{
	Apply_Dissolve(In.vPosition); // 디더 디졸브

	PS_OUT Out;
	Out.vDiffuse = g_vColor; // 텍스처 대신 상수 색
	Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(g_vMRA, 1.f); //상수 MRA
	Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
}

// Decal
struct VS_DECAL_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;

    nointerpolation float4 vWorldRow0 : TEXCOORD1;
    nointerpolation float4 vWorldRow1 : TEXCOORD2;
    nointerpolation float4 vWorldRow2 : TEXCOORD3;

    nointerpolation float4 vWorldInvRow0 : TEXCOORD4;
    nointerpolation float4 vWorldInvRow1 : TEXCOORD5;
    nointerpolation float4 vWorldInvRow2 : TEXCOORD6;
    nointerpolation float4 vWorldInvRow3 : TEXCOORD7;
};

float3x3 Build_WorldInv3x3(float3 vRow0, float3 vRow1, float3 vRow2)
{
    float3 vC0 = cross(vRow1, vRow2);
    float3 vC1 = cross(vRow2, vRow0);
    float3 vC2 = cross(vRow0, vRow1);
    float fDet = dot(vRow0, vC0);
    float fSafeDet = abs(fDet) < 1e-6f ? (fDet < 0.f ? -1e-6f : 1e-6f) : fDet;
    float fInvDet = rcp(fSafeDet);

    return float3x3(
          vC0.x, vC1.x, vC2.x,
          vC0.y, vC1.y, vC2.y,
          vC0.z, vC1.z, vC2.z) * fInvDet;
}

VS_DECAL_OUT VS_DECAL(VS_IN In)
{
    VS_DECAL_OUT Out = (VS_DECAL_OUT) 0;

    float4 vWorld = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
    Out.vPosition = mul(mul(vWorld, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;

    float3 vWorldRow0 = In.WorldMatrix[0].xyz;
    float3 vWorldRow1 = In.WorldMatrix[1].xyz;
    float3 vWorldRow2 = In.WorldMatrix[2].xyz;
    float3 vWorldTrans = In.WorldMatrix[3].xyz;

    float3x3 matWorldInv3 = Build_WorldInv3x3(vWorldRow0, vWorldRow1, vWorldRow2);
    float3 vWorldInvTrans = mul(-vWorldTrans, matWorldInv3);

    Out.vWorldRow0 = In.WorldMatrix[0];
    Out.vWorldRow1 = In.WorldMatrix[1];
    Out.vWorldRow2 = In.WorldMatrix[2];

    Out.vWorldInvRow0 = float4(matWorldInv3[0], 0.f);
    Out.vWorldInvRow1 = float4(matWorldInv3[1], 0.f);
    Out.vWorldInvRow2 = float4(matWorldInv3[2], 0.f);
    Out.vWorldInvRow3 = float4(vWorldInvTrans, 1.f);

    return Out;
}

struct PS_DECAL_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vMRA : SV_TARGET2;
};

PS_DECAL_OUT PS_DECAL(VS_DECAL_OUT In)
{
    PS_DECAL_OUT Out = (PS_DECAL_OUT) 0;

    float2 screenUV;
    screenUV.x = In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f;
    screenUV.y = In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f;

    float sceneDepth = g_DepthTexture.Sample(PointSampler, screenUV).x;
    if (sceneDepth <= 0.f)
        discard;

    float3 worldPosition = RecoverWorldPos(screenUV, sceneDepth, g_ProjMatrixInverse, g_ViewMatrixInverse);

    row_major float4x4 matWorldInv = float4x4(In.vWorldInvRow0, In.vWorldInvRow1, In.vWorldInvRow2, In.vWorldInvRow3);
    float3 decalLocalPosition = mul(float4(worldPosition, 1.f), matWorldInv).xyz;

    float3 boundsDistance = abs(decalLocalPosition - g_vDecalBoundsCenter);
    if (any(boundsDistance > g_vDecalBoundsExtents + 0.0001f))
        discard;

    uint surfMatID = g_MaterialIDTexture.Load(int3(In.vPosition.xy, 0));
    if ((g_iDecalMaskMode == 0 && surfMatID == (uint) g_iDecalMaskID) || (g_iDecalMaskMode != 0 && surfMatID != (uint) g_iDecalMaskID))
        discard;

    float2 decalUV = float2(decalLocalPosition.x + 0.5f, 0.5f - decalLocalPosition.z);
    decalUV = ApplyMeshUVTransform(decalUV);

    float4 col = g_DiffuseTexture.Sample(LinearSampler, decalUV);
    if (col.a <= 0.f)
        discard;

    float coverage = col.a * g_fDecalAlpha;

    Out.vDiffuse = float4(col.rgb, coverage);

    float3 nT = g_NormalTexture.Sample(LinearSampler, decalUV).xyz * 2.f - 1.f;
    float3 T = normalize(In.vWorldRow0.xyz);
    float3 N = normalize(In.vWorldRow1.xyz);
    float3 B = -normalize(In.vWorldRow2.xyz);
    float3 worldN = normalize(nT.x * T + nT.y * B + nT.z * N);
    Out.vNormal = float4(worldN * 0.5f + 0.5f, coverage * g_fDecalHasNormal);

    float3 mra = g_MRATexture.Sample(LinearSampler, decalUV).rgb;
    Out.vMRA = float4(mra, coverage * g_fDecalHasMRA);

    return Out;
}

technique11 DefaultTechnique
{
	pass SHADOW // 0
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_SHADOW();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SHADOW();
	}
	pass White_Pass // 1
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_WHITE();
	}
	pass DIFF_Pass // 2
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DIFF();
	}
	pass DMN_Pass // 3
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DMN();
	}
	pass UKWN_Pass // 4
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_UKWN();
	}
	pass UMN_Pass // 5
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_UMN();
	}
	pass DMNU_Pass // 6
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DMNU();
	}
	pass TREESHADOW_Pass // 7
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_TREESHADOW();
	}
	pass GRASS_FUR_Pass // 8
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_GRASS_FUR();
	}
	pass MN_Pass // 9
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MN();
	}
	pass DISCARD_Pass // 10
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DISCARD();
	}
	pass ColorMRADitherPass // 11
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_COLOR_MRA_DITHER();
	}
	pass Decal_Pass // 12
	{
		SetRasterizerState(RS_Decal);
		SetDepthStencilState(DSS_NoWrite, 0);
		SetBlendState(BS_Decal, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_DECAL(); // 인스턴스 VS 그대로
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DECAL();
	}
}