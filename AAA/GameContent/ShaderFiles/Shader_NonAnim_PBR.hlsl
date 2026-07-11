#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnknownTexture;
Texture2D g_MRATexture;

float4 g_vColor = float4(1.f, 1.f, 1.f, 1.f);
float3 g_vMRA = float3(0.f, 1.f, 1.f);

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);

uint g_iMaterialID = 0;

uint g_iUVIndex = 0;
float4 g_vUVTransform = float4(1.f, 1.f, 0.f, 0.f);
uint g_iUnknownUVIndex = 0;
float4 g_vUVTransformUnknown = float4(1.f, 1.f, 0.f, 0.f);
float g_fUVRotate = 0.f;

float g_fDissolve;

#define SHADOW_ALPHA_NONE 0u
#define SHADOW_ALPHA_DIFFUSE 1u
#define SHADOW_ALPHA_UNKNOWN 2u
#define SHADOW_ALPHA_DISCARD_ALL 3u
#define SHADOW_ALPHA_DIFFUSE_R 4u
#define SHADOW_ALPHA_UNKNOWN_R 5u
uint g_iShadowAlphaSource = SHADOW_ALPHA_NONE;

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

float2 ApplyMeshUVTransformEx(float2 uv, float4 Transform)
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

float2 ApplyMeshUVTransform(float2 uv)
{
    return ApplyMeshUVTransformEx(uv, g_vUVTransform);
}

float2 ApplyUnknownUVTransform(float2 uv)
{
    return ApplyMeshUVTransformEx(uv, g_vUVTransformUnknown);
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

/* 정점셰이더 : 정점 데이터의 변환 과정을 수행한다. */

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out;

	/* 월드변환, 뷰 벼환, 투영변환 */
	float4      vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
	vPosition = mul(vPosition, g_ViewMatrix);
	vPosition = mul(vPosition, g_ProjMatrix);

	Out.vPosition = vPosition;
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
	Out.vTexcoord = In.vTexcoord;
	Out.vTexcoord1 = In.vTexcoord1;
	Out.vTexcoord2 = In.vTexcoord2;
	Out.vTexcoord3 = In.vTexcoord3;
	Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
	Out.vProjPos = Out.vPosition;

	float3 T = normalize(In.vTangent.xyz);
	float3 B = normalize(In.vBinormal.xyz);
	Out.vTangent = normalize(mul(float4(T, 0.f), g_WorldMatrix));
	Out.vTangent.w = In.vTangent.w;
	Out.vBinormal = normalize(mul(float4(B, 0.f), g_WorldMatrix));
	Out.vBinormal.w = In.vBinormal.w;


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

struct PS_BACKOUT
{
	float4 vDiffuse : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT Out;

	vector vEye = g_UnknownTexture.Sample(ClampSampler, In.vTexcoord);
	vector vBase = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord1);
	float3 vAlbedo = lerp(vBase.rgb, vEye.rgb, vEye.a);
	if (vBase.a < 0.1f)
		discard;

	float3 mra = g_MRATexture.Sample(LinearSampler, In.vTexcoord1).rgb;

	float3 N = normalize(In.vNormal);
	float3 T = normalize(In.vTangent.xyz);
	float3 B = normalize(In.vBinormal.xyz);

	float3x3 TBN = float3x3(T, B, N);

	float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord1).rg;
	float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));

	float3 Nw = mul(nTS, TBN);

	Out.vDiffuse = float4(vAlbedo, vBase.a);
	Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    Out.vMaterialID = g_iMaterialID;
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);

	return Out;
}

PS_OUT PS_DIFFUSE(PS_IN In)
{
	PS_OUT Out;

	vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
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

PS_OUT PS_DIFFUSE_DITHER(PS_IN In)
{
	Apply_Dissolve(In.vPosition); // 맨 위에서 디더 판정

	PS_OUT Out;
	vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	float3 N = normalize(In.vNormal);
	float3 T = normalize(In.vTangent.xyz);
	float3 B = normalize(In.vBinormal.xyz);

	float3x3 TBN = float3x3(T, B, N);

	float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord1).rg;
	float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));

	float3 Nw = mul(nTS, TBN);

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(Nw * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
	Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
	Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
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
	float4 vWorld = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
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
	if (SHADOW_ALPHA_DISCARD_ALL == g_iShadowAlphaSource)
		discard;

	float fAlpha = 1.f;

	[branch]
	if (SHADOW_ALPHA_DIFFUSE == g_iShadowAlphaSource)
		fAlpha = g_DiffuseTexture.Sample(LinearSampler, vUV).a;
	else if (SHADOW_ALPHA_UNKNOWN == g_iShadowAlphaSource)
		fAlpha = g_UnknownTexture.Sample(LinearSampler, vUnknownUV).a;
    else if (SHADOW_ALPHA_DIFFUSE_R == g_iShadowAlphaSource)
		fAlpha = g_DiffuseTexture.Sample(LinearSampler, vUV).r;
    else if (SHADOW_ALPHA_UNKNOWN_R == g_iShadowAlphaSource)
        fAlpha = g_UnknownTexture.Sample(LinearSampler, vUnknownUV).r;
	
	if (fAlpha < 0.1f)
		discard;
}

PS_SHADOW_OUT PS_SHADOW(VS_SHADOW_OUT In)
{
    Apply_ShadowAlphaCut(In.vTexcoord, In.vUnknownTexcoord);

	PS_SHADOW_OUT Out;
	float d = In.vProjPos.z / In.vProjPos.w; // 디퍼드의 pz(=lc.z/lc.w)와 동일 공간
    Out.vLightDepth = float4(d, 0.f, 0.f, 1.f);
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
	Out.vMRA = float4(g_vMRA, 1.f); // 상수 MRA
	Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
	Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
	return Out;
}

technique11 DefaultTechnique
{
	pass DefaultPass // 0
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass DiffusePass // 1
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DIFFUSE();
	}

	pass ShadowPass // 2
	{
		SetRasterizerState(RS_Cull_None); // 피터팬 심하면 앞면 컬링 RS로 교체
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_SHADOW();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SHADOW();
	}

	pass WhitePass // 3
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_WHITE();
	}

	pass DitherDiffusePass // 4
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DIFFUSE_DITHER();
	}
	pass DIFF_Pass	// 5
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass DMN_Pass	// 6
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass UKWN_Pass	// 7
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass UMN_Pass	// 8
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass DMNU_Pass // 9
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass TREESHADOW_Pass // 10
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass GRASS_FUR_Pass // 11
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass COLOR_Pass // 12
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass DISCARD_Pass // 13
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
	pass ColorMRADitherPass // 14
	{
		SetRasterizerState(RS_Cull_None);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_COLOR_MRA_DITHER();
	}
	pass Decal_Pass // 15
	{
		SetRasterizerState(RS_Cull_CW);
		SetDepthStencilState(DSS_Z_Disable, 0);
		SetBlendState(BS_Decal, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
	}
    pass COLOR2_Pass // 16
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
}