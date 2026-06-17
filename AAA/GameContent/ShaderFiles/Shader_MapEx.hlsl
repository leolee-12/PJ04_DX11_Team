#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MRATexture;
Texture2D g_UnknownTexture;
Texture2D g_ExtraRTexture;
Texture2D g_ExtraGTexture;
Texture2D g_ExtraBTexture;
Texture2D g_ExtraATexture;

uint   g_iBase_UVIndex = 0;
uint   g_iUnknown_UVIndex = 0;
uint   g_iExtraR_UVIndex = 0;
uint   g_iExtraG_UVIndex = 0;
uint   g_iExtraB_UVIndex = 0;
uint   g_iExtraA_UVIndex = 0;

float  g_NormalStrength = 1.f;
float  g_MaskStrength = 1.f;
float  g_fAOStrength = 1.f;

float4 g_vUVTransform = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformNormal = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformMaterial = float4(1.f, 1.f, 0.f, 0.f);
float  g_fUVRotate = 0.f;
float4 g_vExtraEnable = float4(0.f, 0.f, 0.f, 0.f);
float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);
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
	float4 vColor : COLOR0;     // _c0  tint / blend
	float4 vColor1 : COLOR1;     // _c1  AO / blend
	float4 vColor2 : COLOR2;     // _c2  spare mask
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
	float4 vColor : COLOR0;
	float4 vColor1 : COLOR1;
	float4 vColor2 : COLOR2;
};

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out;

	float4 vWorld = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
	Out.vPosition = mul(mul(vWorld, g_ViewMatrix), g_ProjMatrix);

	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
	Out.vTexcoord = In.vTexcoord;
	Out.vTexcoord1 = In.vTexcoord1;
	Out.vTexcoord2 = In.vTexcoord2;
	Out.vTexcoord3 = In.vTexcoord3;
	Out.vWorldPos = vWorld;
	Out.vProjPos = Out.vPosition;

	Out.vTangent = normalize(mul(float4(In.vTangent.xyz, 0.f), g_WorldMatrix));
	Out.vTangent.w = In.vTangent.w;
	Out.vBinormal = normalize(mul(float4(In.vBinormal.xyz, 0.f), g_WorldMatrix));
	Out.vBinormal.w = In.vBinormal.w;

	Out.vColor = In.vColor;
	Out.vColor1 = In.vColor1;
	Out.vColor2 = In.vColor2;

	return Out;
}

//============================ Shadow (depth-only) ============================
struct VS_SHADOW_OUT
{
	float4 vPosition : SV_POSITION;
	float4 vProjPos : TEXCOORD0;
};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
	VS_SHADOW_OUT Out;
	float4 vWorld = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
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
	float d = In.vProjPos.z / In.vProjPos.w; // 디퍼드의 pz(=lc.z/lc.w)와 동일 공간
	Out.vLightDepth = float4(d, d, d, 1.f);
	return Out;
}

/* -------------------- Main -------------------- */
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
	float4 vColor : COLOR0;
	float4 vColor1 : COLOR1;
	float4 vColor2 : COLOR2;
};

struct PS_OUT
{
	float4 vDiffuse : SV_TARGET0;
	float4 vNormal : SV_TARGET1;
	float4 vDepth : SV_TARGET2;
	float4 vMRA : SV_TARGET3;
	float4 vEmissive : SV_TARGET4;
};

float2 Select_MapUV(PS_IN In, uint iUVIndex)
{
	[branch] if (iUVIndex == 1) return In.vTexcoord1;
	[branch] if (iUVIndex == 2) return In.vTexcoord2;
	[branch] if (iUVIndex == 3) return In.vTexcoord3;
	return In.vTexcoord;
}

float2 Apply_MapUVTransform(float2 uv, float4 Transform)
{
    uv *= Transform.xy;

	float s = sin(g_fUVRotate);
	float c = cos(g_fUVRotate);

	uv = float2(
		uv.x * c - uv.y * s,
		uv.x * s + uv.y * c
	);

    uv += Transform.zw;
	return uv;
}

PS_OUT PS_WHITE(PS_IN In)
{
	PS_OUT Out;
	Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
	Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f); // 노멀맵 없음 → 기하노멀
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, 1.f, 1.f); // metal0 / rough1 / ao1 기본
	Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
	return Out;
}

PS_OUT PS_DIFF(PS_IN In)
{
	PS_OUT Out;
    float2 vBaseUV = Apply_MapUVTransform(Select_MapUV(In, g_iBase_UVIndex), g_vUVTransform);
	float4 vBase = g_DiffuseTexture.Sample(LinearSampler, vBaseUV);
	if (vBase.a < 0.1f)
		discard;

	float3 albedo = vBase.rgb;
	float ao = lerp(1.f, In.vColor1.r, g_fAOStrength);
	albedo *= ao;

	Out.vDiffuse = float4(albedo, 1.f);
	Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, ao, g_iMaterialID / 255.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
	return Out;
}

PS_OUT PS_DN(PS_IN In)
{
	PS_OUT Out;
	float2 vBaseUV = Apply_MapUVTransform(Select_MapUV(In, g_iBase_UVIndex), g_vUVTransform);
    float2 vNormalUV = Apply_MapUVTransform(Select_MapUV(In, g_iBase_UVIndex), g_vUVTransformNormal);
	float4 vBase = g_DiffuseTexture.Sample(LinearSampler, vBaseUV);
	if (vBase.a < 0.1f)
		discard;

	float3 albedo = vBase.rgb;

	float3 N = normalize(In.vNormal.xyz);
	float3 T = normalize(In.vTangent.xyz);
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T) * In.vTangent.w;
	float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg * 2.f - 1.f;
	nrg *= g_NormalStrength;
	float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
	nTS.y = -nTS.y;
	float3 Nw = normalize(mul(nTS, TBN));

	float ao = lerp(1.f, In.vColor1.r, g_fAOStrength);
	albedo *= ao;

	Out.vDiffuse = float4(albedo, 1.f);
	Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, ao, g_iMaterialID / 255.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
	return Out;
}

PS_OUT PS_DMN(PS_IN In)
{
	PS_OUT Out;
	float2 vBaseUV = Apply_MapUVTransform(Select_MapUV(In, g_iBase_UVIndex), g_vUVTransform);
    float2 vNormalUV = Apply_MapUVTransform(Select_MapUV(In, g_iBase_UVIndex), g_vUVTransformNormal);
    float2 vMaterialUV = Apply_MapUVTransform(Select_MapUV(In, g_iBase_UVIndex), g_vUVTransformMaterial);
	float4 vBase = g_DiffuseTexture.Sample(LinearSampler, vBaseUV);
	if (vBase.a < 0.1f)
		discard;

	float3 albedo = vBase.rgb;
    float3 mra = g_MRATexture.Sample(LinearSampler, vMaterialUV).rgb;

	float3 N = normalize(In.vNormal.xyz);
	float3 T = normalize(In.vTangent.xyz);
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T) * In.vTangent.w;
	float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg * 2.f - 1.f;
	nrg *= g_NormalStrength;
	float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
	nTS.y = -nTS.y;
	float3 Nw = normalize(mul(nTS, TBN));

	float ao = lerp(1.f, In.vColor1.r, g_fAOStrength);
	albedo *= ao;
	mra.b *= ao;

	Out.vDiffuse = float4(albedo, 1.f);
	Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(mra, g_iMaterialID / 255.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
	return Out;
}

PS_OUT PS_DMNU(PS_IN In)
{
	PS_OUT Out = PS_DMN(In);
	float2 vUnknownUV = Apply_MapUVTransform(Select_MapUV(In, g_iUnknown_UVIndex), g_vUVTransform);
	float4 vUnknown = g_UnknownTexture.Sample(LinearSampler, vUnknownUV);
	return Out;
}

technique11 DefaultTechnique
{
	pass Shadow // 0
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_SHADOW();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SHADOW();
	}
	pass White	// 1
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_WHITE();
	}
	pass DIFF	// 2
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DIFF();
	}
	pass DN		// 3
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DN();
	}
	pass DMN	// 4
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DMN();
	}
	pass DMNU	// 5
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DMNU();
	}
}
