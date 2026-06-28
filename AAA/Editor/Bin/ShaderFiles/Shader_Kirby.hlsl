#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_BoneMatrices[512];

Texture2D g_EyeTexture; // _a0  KirbyEye.0X      (표정 프레임)
Texture2D g_SkinTexture; // _a1  KirbySkin        (영역 마스크! 알베도 아님)
Texture2D g_MouthTexture; // _a2  KirbyMouth
Texture2D g_EyeMaskTexture; // _a3  KirbyEyeMask.0X  (눈 합성 마스크)
Texture2D g_WetMaskTexture; // _a4  WetMask          (머금기/젖음)
Texture2D g_WarpTexture; // _a5  Warp             (흡입 왜곡, 옵션/미사용)
Texture2D g_NormalTexture; // _n0  KirbyEyeNormal.0X

float4 g_vBodyColor = float4(1.f, 0.45f, 0.55f, 1.f); // 몸  = KirbySkin 검정 영역
float4 g_vFootColor = float4(1.f, 0.1882353f, 0.3764706f, 1.f); // 발  = G 채널 초록
float4 g_vBlushColor = float4(1.f, 0.25f, 0.4f, 1.f); // 홍조 = R 채널 빨강
float4 g_vDamageColor = float4(1.f, 0.f, 0.f, 1.f); // 피격 플래시
float g_fDamageBlend = 0.f; // 0~1
float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);

float g_fWetStrength = 0.f;
uint g_iMaterialID = 0;

static const float3 EYE_WHITE = float3(1.f, 1.f, 1.f);
static const float3 EYE_BLUE = float3(0.12f, 0.45f, 1.f);
static const float3 EYE_RIM = float3(0.1f, 0.1f, 0.1f);

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

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    float4x4 BoneMatrix =
            g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x
          + g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y
          + g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z
          + g_BoneMatrices[In.vBlendIndex.w] * fWeightW;

    float4 vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    float4 vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);

    float4x4 matWV = mul(g_WorldMatrix, g_ViewMatrix);
    float4x4 matWVP = mul(matWV, g_ProjMatrix);

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

PS_OUT PS_BODY(PS_IN In)
{
    PS_OUT Out;

    float4 vMask = g_SkinTexture.Sample(ClampSampler, In.vTexcoord2);
    float3 vAlbedo = g_vBodyColor.rgb;
    vAlbedo = lerp(vAlbedo, g_vFootColor.rgb, vMask.g);
    vAlbedo = lerp(vAlbedo, g_vBlushColor.rgb, vMask.r);
    
    // 2) 눈 합성 (EyeMask 명도 × Eye 알파로 스킨 위에 덮음)
    float fEyeAlpha = g_EyeTexture.Sample(ClampSampler, In.vTexcoord).r;
    float3 vEyeMask = g_EyeMaskTexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 result =
            vEyeMask.r * EYE_WHITE +
            vEyeMask.g * EYE_RIM +
            vEyeMask.b * EYE_BLUE;
    
    
    vAlbedo = lerp(vAlbedo, result, fEyeAlpha);
    
    // 3) 입 합성
    float4 vMouth = g_MouthTexture.Sample(ClampSampler, In.vTexcoord1);
    vAlbedo = lerp(vAlbedo, vMouth.rgb, vMouth.a);
    

    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(In.vNormal.rgb * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 0.7f, 1.f, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;

    return Out;
}

PS_OUT PS_MASK_DEBUG(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float4 vMask = g_SkinTexture.Sample(LinearSampler, In.vTexcoord1);
    Out.vDiffuse = float4(vMask.rgb, 1.f);
    Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

technique11 DefaultTechnique
{
    pass Body // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_MarkOccluded, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BODY();
    }

    pass MaskDebug // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MASK_DEBUG();
    }
}