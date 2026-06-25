#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnknownTexture;
Texture2D g_MRATexture;

float4 g_vColor = float4(1.f, 1.f, 1.f, 1.f);
float3 g_vMRA = float3(0.f, 1.f, 1.f);

float2 g_vMaskValue;
float4 g_vBlendColor;

float g_NormalStrength = 1.f;

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);

uint g_iMaterialID = 0;

uint g_iUVIndex = 0;
float4 g_vUVTransform = float4(1.f, 1.f, 0.f, 0.f);

#define ENV_INSTANCE_FLAG_DITHER 0x01
uint g_iEnvInstanceFlags = 0;
float g_fDissolve;

#define ENV_SHADOW_ALPHA_NONE 0u
#define ENV_SHADOW_ALPHA_DIFFUSE 1u
#define ENV_SHADOW_ALPHA_UNKNOWN 2u
#define ENV_SHADOW_ALPHA_DISCARD_ALL 3u
uint g_iShadowAlphaSource = ENV_SHADOW_ALPHA_NONE;

static const float Bayer4x4[16] =
{
    0.0  / 16.0, 8.0  / 16.0, 2.0  / 16.0, 10.0 / 16.0,
    12.0 / 16.0, 4.0  / 16.0, 14.0 / 16.0, 6.0  / 16.0,
    3.0  / 16.0, 11.0 / 16.0, 1.0  / 16.0, 9.0  / 16.0,
    15.0 / 16.0, 7.0  / 16.0, 13.0 / 16.0, 5.0  / 16.0
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

struct VS_NONINST_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;

    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

float2 Select_UV_PS(VS_IN In)
{
    [branch] if (1 == g_iUVIndex) return In.vTexcoord1;
    [branch] if (2 == g_iUVIndex) return In.vTexcoord2;
    [branch] if (3 == g_iUVIndex) return In.vTexcoord3;
    return In.vTexcoord;
}

VS_NONINST_OUT VS_NONINST_MAIN(VS_IN In)
{
    VS_NONINST_OUT Out;

    /* 월드변환, 뷰 벼환, 투영변환 */
    float4      vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vPosition = mul(vPosition, g_ViewMatrix);
    vPosition = mul(vPosition, g_ProjMatrix);

    Out.vPosition = vPosition;
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = ApplyMeshUVTransform(Select_UV_PS(In));
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
};

struct PS_BACKOUT
{
    float4 vDiffuse : SV_TARGET0;
};

// Non-Instanced
struct PS_NONINST_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;

    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
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
    

/* 픽셀셰이더 : 픽셀의 최종적인 색을 결정해준다. */
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
    Out.vMRA = float4(mra, g_iMaterialID / 255.f);
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
    Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
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
    Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    return Out;
}

PS_OUT PS_WHITE(PS_IN In)
{
    PS_OUT Out;

    Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
    Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
    Out.vEmissive = float4(0.f, 0.f, 0.f, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);

    return Out;
}

//============================ Shadow (depth-only) ============================
struct VS_SHADOW_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
    float2 vTexcoord : TEXCOORD1;
};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
    VS_SHADOW_OUT Out;
    float4 vWorld = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vPosition = mul(mul(vWorld, g_ViewMatrix), g_ProjMatrix);
    Out.vProjPos = Out.vPosition;
    Out.vTexcoord = ApplyMeshUVTransform(Select_UV_PS(In));
    return Out;
}

struct PS_SHADOW_OUT
{
    float4 vLightDepth : SV_TARGET0;
};

void Apply_ShadowAlphaCut(float2 vUV)
{
      [branch]
    if (ENV_SHADOW_ALPHA_DISCARD_ALL == g_iShadowAlphaSource)
        discard;

    float fAlpha = 1.f;

      [branch]
    if (ENV_SHADOW_ALPHA_DIFFUSE == g_iShadowAlphaSource)
        fAlpha = g_DiffuseTexture.Sample(LinearSampler, vUV).a;
    else if (ENV_SHADOW_ALPHA_UNKNOWN == g_iShadowAlphaSource)
        fAlpha = g_UnknownTexture.Sample(LinearSampler, vUV).a;

    if (fAlpha < 0.1f)
        discard;
}

PS_SHADOW_OUT PS_SHADOW(VS_SHADOW_OUT In)
{
    Apply_ShadowAlphaCut(In.vTexcoord);
    
    PS_SHADOW_OUT Out;
    float d = In.vProjPos.z / In.vProjPos.w; // 디퍼드의 pz(=lc.z/lc.w)와 동일 공간
    Out.vLightDepth = float4(d, d, d, 1.f);
    return Out;
}

/* Non-Instanced */
PS_OUT PS_DIFF_SAMPLE(PS_NONINST_IN In, float2 vUV)
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
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    return Out;
}

PS_OUT PS_DMN_SAMPLE(PS_NONINST_IN In, float2 vUV)
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
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    return Out;
}

PS_OUT PS_UKWN_SAMPLE(PS_NONINST_IN In, float2 vUV)
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
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    return Out;
}

PS_OUT PS_UMN_SAMPLE(PS_NONINST_IN In, float2 vUV)
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
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    return Out;
}

PS_OUT PS_DIFF(PS_NONINST_IN In)
{
    return PS_DIFF_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_DMN(PS_NONINST_IN In)
{
    return PS_DMN_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_UKWN(PS_NONINST_IN In)
{
    return PS_UKWN_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_UMN(PS_NONINST_IN In)
{
    return PS_UMN_SAMPLE(In, In.vTexcoord);
}

PS_OUT PS_DMNU(PS_NONINST_IN In)
{
    PS_OUT Out = PS_DIFF_SAMPLE(In, In.vTexcoord);
	
    return Out;
}

PS_OUT PS_TREESHADOW(PS_NONINST_IN In)
{
    PS_OUT Out = PS_DIFF_SAMPLE(In, In.vTexcoord);
	
    return Out;
}

PS_OUT PS_GRASS_FUR(PS_NONINST_IN In)
{
    PS_OUT Out = PS_DIFF_SAMPLE(In, In.vTexcoord);
	
    return Out;
}

PS_OUT PS_MN(PS_NONINST_IN In)
{
    Apply_Dither_IfNeeded(In.vPosition);
	
    PS_OUT Out;

    float3 vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

    Out.vDiffuse = vector(0.294f, 0.424f, 0.235f, 1.f);
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, g_iMaterialID / 255.f);
    Out.vEmissive = vector(0.f, 0.f, 0.f, 0.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    return Out;
}

PS_OUT PS_DISCARD(PS_NONINST_IN In)
{
    discard;
    return (PS_OUT) 0;
}

PS_OUT PS_COLOR_MRA_DITHER(PS_IN In)
{
    Apply_Dissolve(In.vPosition); // 디더 디졸브

    PS_OUT Out;
    Out.vDiffuse = g_vColor; // 텍스처 대신 상수 색
    Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(g_vMRA, g_iMaterialID / 255.f); // 상수 MRA
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
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

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DIFF();
    }
    pass DMN_Pass	// 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DMN();
    }
    pass UKWN_Pass	// 7
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_UKWN();
    }
    pass UMN_Pass	// 8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_UMN();
    }
    pass DMNU_Pass // 9
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DMNU();
    }
    pass TREESHADOW_Pass // 10
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TREESHADOW();
    }
    pass GRASS_FUR_Pass // 11
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRASS_FUR();
    }
    pass MN_Pass // 12
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MN();
    }
    pass DISCARD_Pass // 13
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_NONINST_MAIN();
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
}