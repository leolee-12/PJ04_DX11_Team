#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_BoneMatrices[512];

Texture2D g_DiffuseTexture;
Texture2D g_MRATexture;
Texture2D g_NormalTexture;
Texture2D g_BodyMaskTexture;
Texture2D g_RockHoleMaskTexture;

float g_fDissolve;

float4 g_vEmissiveColor;
uint g_iMaterialID = 0;

float g_fShadowDepthBias = 0.075f;

static const float3 EYE_WHITE = float3(1.f, 1.f, 1.f);
static const float3 EYE_BLUE = float3(0.12f, 0.45f, 1.f);
static const float3 EYE_RIM = float3(0.1f, 0.1f, 0.1f);

static const float Bayer4x4[16] =
{
    0.0 / 16.0, 8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0,
	12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0, 6.0 / 16.0,
	3.0 / 16.0, 11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0,
	15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0, 5.0 / 16.0
};

float g_fHitFlash = 0.f;
float3 g_vHitFlashColor = float3(1.f, 1.f, 1.f);

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

//======== Shadow (depth-only, skinned) ========
struct VS_SHADOW_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

VS_SHADOW_OUT VS_SHADOW(VS_IN In)
{
    VS_SHADOW_OUT Out;
    float fW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    float4x4 Bone = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x
                  + g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y
                  + g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z
                  + g_BoneMatrices[In.vBlendIndex.w] * fW;
    float4 vPos = mul(float4(In.vPosition, 1.f), Bone);
    float4 vWorld = mul(vPos, g_WorldMatrix);
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
    float d = In.vProjPos.z / In.vProjPos.w;
    d += g_fShadowDepthBias; // 오클루더를 광원에서 살짝 밀어 self-shadow 제거
    Out.vLightDepth = float4(d, d, d, 1.f);
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

    float4 vDiffuse = g_BodyMaskTexture.Sample(ClampSampler, In.vTexcoord);
    float  fGray = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord).r;
    
    float3 vGrayPart = vDiffuse.rgb * fGray;
    
    float3 vAlbedo = lerp(vGrayPart, vDiffuse.rgb, vDiffuse.a);
    
    float3 vMRA = g_MRATexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);
    
   
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vEmissive.rgb += g_vHitFlashColor * g_fHitFlash;
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;

    return Out;
}

PS_OUT PS_EYE(PS_IN In)
{
    PS_OUT Out;

    
    float fEyeAlpha = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord).a;
    float3 vEyeMask = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 vAlbedo = float3(1.f, 1.f, 1.f);
    
    float3 result =
            vEyeMask.r * EYE_WHITE +
            vEyeMask.g * EYE_RIM +
            vEyeMask.b * EYE_BLUE;

    vAlbedo = lerp(vAlbedo, result, fEyeAlpha);
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);
   
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vEmissive.rgb += g_vHitFlashColor * g_fHitFlash;
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;

    return Out;
}

PS_OUT PS_SKIN(PS_IN In)
{
    PS_OUT Out;

    float3 vAlbedo = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord).rgb;
    float3 vMRA    = g_MRATexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);
   
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vEmissive.rgb += g_vHitFlashColor * g_fHitFlash;
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;

    return Out;
}

PS_OUT PS_ANCHOR(PS_IN In)
{
    PS_OUT Out;

    float3 vAlbedo = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord).rgb;
    float3 vMRA = g_MRATexture.Sample(ClampSampler, In.vTexcoord).rgb;
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);
   
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vEmissive.rgb += g_vHitFlashColor * g_fHitFlash;
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;

    return Out;
}

PS_OUT PS_ROCKHOLE(PS_IN In)
{
    PS_OUT Out;
    
    Apply_Dissolve(In.vPosition);

    float3 vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float3 vMask = g_RockHoleMaskTexture.Sample(ClampSampler, In.vTexcoord1).rgb;
    float3 vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;
    
    float3 vAlbedo = mul(vDiffuse, vMask);
    
    float3 N = normalize(In.vNormal);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);
  
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    
    float3 Nw = mul(nTS, TBN);
   
    Out.vDiffuse = float4(vAlbedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(vMRA, 1.f);
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
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BODY();
    }
    pass Eye // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EYE();
    }
    pass Skin // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SKIN();
    }
    pass Anchor // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ANCHOR();
    }
    pass RockHole // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ROCKHOLE();
    }
    pass ShadowPass // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
}