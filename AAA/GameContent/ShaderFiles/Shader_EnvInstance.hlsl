#include "Engine_Shader_Defines.hlsli"

float4x4 g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnknownTexture;
Texture2D g_MRATexture;

float2 g_vMaskValue;
float4 g_vBlendColor;

float g_NormalStrength = 1.f;

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);

uint g_iMaterialID = 0;

// 0=TEXCOORD0, 1=TEXCOORD1, 2=TEXCOORD2, 3=TEXCOORD3
uint g_iUVIndex = 0;
float4 g_vUVTransform = float4(1.f, 1.f, 0.f, 0.f);

#define ENV_INSTANCE_FLAG_DITHER 0x01
uint g_iEnvInstanceFlags = 0;

float g_fDissolve;

static const float Bayer4x4[16] =
{
	0.0 / 16.0,	8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0,
	12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0, 6.0 / 16.0,
	3.0 / 16.0,	11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0,
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

	float4 vWorldPos : TEXCOORD1;
	float4 vProjPos : TEXCOORD2;
	float4 vTangent : TANGENT;
	float4 vBinormal : BINORMAL;
};

float2 Select_UV_VS(VS_IN In)
{
	[branch] if (1 == g_iUVIndex) return In.vTexcoord1;
	[branch] if (2 == g_iUVIndex) return In.vTexcoord2;
	[branch] if (3 == g_iUVIndex) return In.vTexcoord3;
	return In.vTexcoord;
}

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out;

	float4 vPosition = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
	vPosition = mul(vPosition, g_ViewMatrix);
	vPosition = mul(vPosition, g_ProjMatrix);

	Out.vPosition = vPosition;
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), In.WorldMatrix));
	Out.vTexcoord = ApplyMeshUVTransform(Select_UV_VS(In));
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
};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
	VS_SHADOW_OUT Out;

	float4 vWorld = mul(float4(In.vPosition, 1.f), In.WorldMatrix);
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

// Main
struct PS_IN
{
	float4 vPosition : SV_POSITION;
	float4 vNormal : NORMAL;
	float2 vTexcoord : TEXCOORD0;

	float4 vWorldPos : TEXCOORD1;
	float4 vProjPos : TEXCOORD2;
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

PS_OUT PS_WHITE(PS_IN In)
{
	PS_OUT Out;

	Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
	Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
	Out.vEmissive = float4(0.f, 0.f, 0.f, 1.f);

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
	Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	return Out;
}

PS_OUT PS_DMN_SAMPLE(PS_IN In, float2 vUV)
{
	Apply_Dither_IfNeeded(In.vPosition);
	
	PS_OUT Out;
	vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, vUV);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	float3 vMRA = g_MRATexture.Sample(LinearSampler, vUV).rgb;

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(vMRA, g_iMaterialID / 255.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
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
	Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	return Out;
}

PS_OUT PS_UMN_SAMPLE(PS_IN In, float2 vUV)
{
	Apply_Dither_IfNeeded(In.vPosition);
	
	PS_OUT Out;
	vector vMtrlDiffuse = g_UnknownTexture.Sample(LinearSampler, vUV);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	float3 vMRA = g_MRATexture.Sample(LinearSampler, vUV).rgb;

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(vMRA, g_iMaterialID / 255.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	return Out;
}

PS_OUT PS_DIFF(PS_IN In)
{
	return PS_DIFF_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_DMN(PS_IN In)
{
	return PS_DMN_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_UKWN(PS_IN In)
{
	return PS_UKWN_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_UMN(PS_IN In)
{
	return PS_UMN_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_DMNU(PS_IN In)
{
    PS_OUT Out = PS_DIFF_SAMPLE(In, In.vTexcoord);
	
    return Out;
}

PS_OUT PS_TREESHADOW(PS_IN In)
{
    PS_OUT Out = PS_DIFF_SAMPLE(In, In.vTexcoord);
	
    return Out;
}

PS_OUT PS_GRASS_FUR(PS_IN In)
{
    PS_OUT Out = PS_DIFF_SAMPLE(In, In.vTexcoord);
	
    return Out;
}

PS_OUT PS_MN(PS_IN In)
{
    Apply_Dither_IfNeeded(In.vPosition);
	
    PS_OUT Out;

    float3 vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

    Out.vDiffuse = vector(0.294f, 0.424f, 0.235f, 1.f);
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, g_iMaterialID / 255.f);
    Out.vEmissive = vector(0.f, 0.f, 0.f, 0.f);
    return Out;
}

technique11 DefaultTechnique
{
	pass SHADOW		// 0
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_SHADOW();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SHADOW();
	}
	pass White_Pass	// 1
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_WHITE();
	}
	pass DIFF_Pass	// 2
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DIFF();
	}
	pass DMN_Pass	// 3
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DMN();
	}
	pass UKWN_Pass	// 4
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_UKWN();
	}
	pass UMN_Pass	// 5
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
}