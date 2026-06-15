#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnknownTexture;
Texture2D g_MRATexture;

float2 g_vMaskValue;
float4 g_vBlendColor;

float g_NormalStrength = 1.f;

float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);

uint g_iMaterialID = 0;

float g_fDissolve;

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

/* w 나누기 연산 : 2차원 투영스페이스로의 변환. */
/* 뷰포트로의 변환 (윈도우좌표로의 변환) */
/* 래스터라이즈 (정점정보를 기반으로 해서 픽셀의 정보를 생성한다. ) */

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

    //Out.vDiffuse = float4(vAlbedo, vBase.a);
    Out.vDiffuse = float4((In.vTangent.w * 0.5f + 0.5f).rrr, 1.f);
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, g_iMaterialID / 255.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    
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
    return Out;
}

PS_OUT PS_DIFFUSE_DITHER(PS_IN In)
{
    Apply_Dissolve(In.vPosition); // 맨 위에서 디더 판정

    PS_OUT Out;
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, g_iMaterialID / 255.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vMtrlDiffuse.a, 1.f);
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
}