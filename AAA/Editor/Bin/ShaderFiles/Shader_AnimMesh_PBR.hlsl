#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_UnkownTexture;
Texture2D g_MRATexture;

float4x4 g_BoneMatrices[512];

float2 g_vMaskValue;
float g_fHullThickness;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;
    
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    
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
    
    return Out;
}

VS_OUT VS_TEST(VS_IN In)
{
    VS_OUT Out;
    
    //float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    
    //float4x4 m0 = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x;
    //float4x4 m1 = g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y;
    //float4x4 m2 = g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z;
    //float4x4 m3 = g_BoneMatrices[In.vBlendIndex.w] * fWeightW;

    //float4x4 BoneMatrix = m0 + m1 + m2 + m3;

    
    /* 월드변환, 뷰 벼환, 투영변환 */ 
    //float4 vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    //float4 vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    float4 vPosition = float4(In.vPosition.rgb, 1.f);
    float4 vNormal = float4(In.vNormal.rgb, 0.f);

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

VS_OUT_HULL VS_OUTLINE_HULL(VS_IN In)
{
    VS_OUT_HULL Out;

    float fW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    float4x4 BoneMatrix =
          g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
          g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
          g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
          g_BoneMatrices[In.vBlendIndex.w] * fW;

    float4 vPos = mul(float4(In.vPosition, 1.f), BoneMatrix);
    float4 vNor = mul(float4(In.vNormal, 0.f), BoneMatrix);

      // 오브젝트 공간에서 노말 방향으로 부풀림
    vPos.xyz += normalize(vNor.xyz) * g_fHullThickness;

    float4x4 matWV = mul(g_WorldMatrix, g_ViewMatrix);
    float4x4 matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(vPos, matWVP);
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
};
    

/* 픽셀셰이더 : 픽셀의 최종적인 색을 결정해준다. */
PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    vector vEye = g_UnkownTexture.Sample(ClampSampler, In.vTexcoord);
    vector vBase = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord1);

    float3 vAlbedo = lerp(vBase.rgb, vEye.rgb, vEye.a);
    vector vMtrlDiffuse = vector(vAlbedo, vBase.a);

    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_NONEYE(PS_IN In)
{
    PS_OUT Out;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    
    float3 mra = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

    if (vMtrlDiffuse.a < 0.1f)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    
    return Out;
}

PS_OUT PS_TEST(PS_IN In)
{
    PS_OUT Out;

    Out.vDiffuse = vector(1.f, 1.f, 1.f, 1.f);
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.f, 0.f, 0.f);
    
    return Out;
}

struct PS_OUT_SHADOW
{
    float4 vLightDepth : SV_TARGET0;
};

PS_OUT_SHADOW PS_MAIN_SHADOW(PS_IN In)
{
    PS_OUT_SHADOW Out;
    
    Out.vLightDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    
    return Out;
}

struct PS_OUT_OUTLINEMASK
{
    float4 vMask : SV_TARGET0;
};

PS_OUT_OUTLINEMASK PS_MAIN_OUTLINEMASK(PS_IN In)
{
    PS_OUT_OUTLINEMASK Out;
    Out.vMask = float4(g_vMaskValue.x, g_vMaskValue.y, 0.f, 0.f);
    return Out;
}

struct PS_OUT_HULL
{
    float4 vBackBuffer : SV_TARGET0;
};

PS_OUT_HULL PS_OUTLINE_HULL(VS_OUT_HULL In)
{
    PS_OUT_HULL Out;
    Out.vBackBuffer = float4(0.f, 0.f, 0.f, 1.f);
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

    pass NonEyePass // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NONEYE();
    }

    pass Test // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_TEST();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TEST();
    }
}