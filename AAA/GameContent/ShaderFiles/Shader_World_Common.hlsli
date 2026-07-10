#include "Engine_Shader_Defines.hlsli"

float4x4 g_ViewMatrix, g_ProjMatrix;
float4x4 g_ViewMatrixInverse, g_ProjMatrixInverse;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MRATexture;
Texture2D g_UnknownTexture;

float4 g_vColor = float4(1.f, 1.f, 1.f, 1.f);
float3 g_vMRA = float3(0.f, 1.f, 1.f);
float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);
float g_NormalStrength = 1.f;
float g_MaskStrength = 1.f;

uint g_iUVIndex = 0;
uint g_iUnknownUVIndex = 0;

float4 g_vUVTransform = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformNormal = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformMaterial = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformUnknown = float4(1.f, 1.f, 0.f, 0.f);
float g_fUVRotate = 0.f;

#define FLAG_DITHER 0x01u

uint g_iFlags = 0u;
float g_fDissolve = 0.f;

uint g_iMaterialID = 0u;
uint g_iShadowAlphaSource = 0u;

#define SHADOW_ALPHA_NONE 0u
#define SHADOW_ALPHA_DIFFUSE 1u
#define SHADOW_ALPHA_UNKNOWN 2u
#define SHADOW_ALPHA_DISCARD_ALL 3u
#define SHADOW_ALPHA_DIFFUSE_R 4u
#define SHADOW_ALPHA_UNKNOWN_R 5u

Texture2D g_DepthTexture;
Texture2D<uint> g_MaterialIDTexture;

float3 g_vDecalBoundsCenter = float3(0.f, 0.f, 0.f);
float3 g_vDecalBoundsExtents = float3(0.5f, 0.5f, 0.5f);
float g_fDecalAlpha = 1.f;

float g_fDecalHasNormal = 0.f;
float g_fDecalHasMRA = 0.f;

int g_iDecalMaskMode = 0;
int g_iDecalMaskID = 200;

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;

    float2 vTexcoord0 : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;

    float4 vProjPos : TEXCOORD4;

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
    uint vMaterialID : SV_TARGET6;
};

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

void Apply_DitherIfNeeded(float4 vScreenPos)
{
    [branch]
    if (0u != (g_iFlags & FLAG_DITHER))
        Apply_Dissolve(vScreenPos);
}

float2 Select_UV(float2 vTexcoord0, float2 vTexcoord1, float2 vTexcoord2, float2 vTexcoord3, uint iUVIndex)
{
    [branch] if (1u == iUVIndex) return vTexcoord1;
    [branch] if (2u == iUVIndex) return vTexcoord2;
    [branch] if (3u == iUVIndex) return vTexcoord3;
    
    return vTexcoord0;
}

float2 Select_UV_PS(PS_IN In, uint iUVIndex)
{
    return Select_UV(In.vTexcoord0, In.vTexcoord1, In.vTexcoord2, In.vTexcoord3, iUVIndex);
}

float2 Apply_UVTransform(float2 uv, float4 Transform)
{
    uv *= Transform.xy;

    [branch]
    if (0.f != g_fUVRotate)
    {
        float s = sin(g_fUVRotate);
        float c = cos(g_fUVRotate);
        uv = float2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
    }

    uv += Transform.zw;
    return uv;
}

float2 Get_BaseUV(PS_IN In)
{
    return Apply_UVTransform(Select_UV_PS(In, g_iUVIndex), g_vUVTransform);
}

float2 Get_NormalUV(PS_IN In)
{
    return Apply_UVTransform(Select_UV_PS(In, g_iUVIndex), g_vUVTransformNormal);
}

float2 Get_MaterialUV(PS_IN In)
{
    return Apply_UVTransform(Select_UV_PS(In, g_iUVIndex), g_vUVTransformMaterial);
}

float2 Get_UnknownUV(PS_IN In)
{
    return Apply_UVTransform(Select_UV_PS(In, g_iUnknownUVIndex), g_vUVTransformUnknown);
}

float3 Reconstruct_Normal(PS_IN In, float2 vNormalUV)
{
    float3 N = normalize(In.vNormal.xyz);
    float3 T = normalize(In.vTangent.xyz);
    float3 B = normalize(In.vBinormal.xyz);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg * g_NormalStrength;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    return mul(nTS, float3x3(T, B, N));
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
    if (SHADOW_ALPHA_DIFFUSE == g_iShadowAlphaSource)           fAlpha = g_DiffuseTexture.Sample(LinearSampler, vUV).a;
    else if (SHADOW_ALPHA_UNKNOWN == g_iShadowAlphaSource)      fAlpha = g_UnknownTexture.Sample(LinearSampler, vUnknownUV).a;
    else if (SHADOW_ALPHA_DIFFUSE_R == g_iShadowAlphaSource)    fAlpha = g_DiffuseTexture.Sample(LinearSampler, vUV).r;
    else if (SHADOW_ALPHA_UNKNOWN_R == g_iShadowAlphaSource)    fAlpha = g_UnknownTexture.Sample(LinearSampler, vUnknownUV).r;

    if (fAlpha < 0.5f)
        discard;
}

PS_SHADOW_OUT Make_ShadowOutput(float4 vProjPos)
{
    PS_SHADOW_OUT Out;
    float d = vProjPos.z / vProjPos.w;
    Out.vLightDepth = float4(d, 0.f, 0.f, 1.f);
    return Out;
}



float4 Encode_Normal(float3 vNormal)
{
    return float4(normalize(vNormal) * 0.5f + 0.5f, 0.f);
}

float4 Encode_Depth(float4 vProjPos)
{
    return float4(vProjPos.z / vProjPos.w, 0.f, 0.f, 0.f);
}

PS_OUT Make_GBufferOutput(PS_IN In, float4 vDiffuse, float3 vNormal, float4 vMRA, float4 vEmissive)
{
    PS_OUT Out;
    Out.vDiffuse = vDiffuse;
    Out.vNormal = Encode_Normal(vNormal);
    Out.vDepth = Encode_Depth(In.vProjPos);
    Out.vMRA = vMRA;
    Out.vEmissive = vEmissive;
    Out.vGeoNormal = Encode_Normal(In.vNormal.xyz);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_WHITE(PS_IN In)
{
    return Make_GBufferOutput(
          In,
          float4(1.f, 1.f, 1.f, 1.f),
          In.vNormal.xyz,
          float4(0.f, 1.f, 1.f, 1.f),
          float4(0.f, 0.f, 0.f, 1.f));
}

PS_OUT PS_DIFF_SAMPLE(PS_IN In, float2 vUV)
{
    Apply_DitherIfNeeded(In.vPosition);

    float4 vDiffuse = g_DiffuseTexture.Sample(LinearSampler, vUV);
    if (vDiffuse.a < 0.1f)
        discard;

    return Make_GBufferOutput(
          In,
          vDiffuse,
          In.vNormal.xyz,
          float4(0.f, 1.f, 1.f, 1.f),
          float4(g_vEmissiveColor.rgb * vDiffuse.a, 1.f));
}

PS_OUT PS_DMN_SAMPLE(PS_IN In, float2 vBaseUV, float2 vNormalUV, float2 vMaterialUV)
{
    Apply_DitherIfNeeded(In.vPosition);

    float4 vDiffuse = g_DiffuseTexture.Sample(LinearSampler, vBaseUV);
    if (vDiffuse.a < 0.1f)
        discard;

    float3 vMRA = g_MRATexture.Sample(LinearSampler, vMaterialUV).rgb;
    float3 vNormal = Reconstruct_Normal(In, vNormalUV);

    return Make_GBufferOutput(
          In,
          vDiffuse,
          vNormal,
          float4(vMRA, 1.f),
          float4(g_vEmissiveColor.rgb * vDiffuse.a, 1.f));
}

PS_OUT PS_UKWN_SAMPLE(PS_IN In, float2 vUV)
{
    Apply_DitherIfNeeded(In.vPosition);

    float4 vDiffuse = g_UnknownTexture.Sample(LinearSampler, vUV);
    if (vDiffuse.a < 0.1f)
        discard;

    return Make_GBufferOutput(
            In,
            vDiffuse,
            In.vNormal.xyz,
            float4(0.f, 1.f, 1.f, 1.f),
            float4(g_vEmissiveColor.rgb * vDiffuse.a, 1.f));
}

PS_OUT PS_UMN_SAMPLE(PS_IN In, float2 vUnknownUV, float2 vNormalUV, float2 vMaterialUV)
{
    Apply_DitherIfNeeded(In.vPosition);

    float4 vDiffuse = g_UnknownTexture.Sample(LinearSampler, vUnknownUV);
    if (vDiffuse.a < 0.1f)
        discard;

    float3 vMRA = g_MRATexture.Sample(LinearSampler, vMaterialUV).rgb;
    float3 vNormal = Reconstruct_Normal(In, vNormalUV);

    return Make_GBufferOutput(
            In,
            vDiffuse,
            vNormal,
            float4(vMRA, 1.f),
            float4(g_vEmissiveColor.rgb * vDiffuse.a, 1.f));
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

    return PS_DMN_SAMPLE(In, Get_BaseUV(In), Get_NormalUV(In), Get_MaterialUV(In));
}

PS_OUT PS_TREESHADOW(PS_IN In)
{
    float fMask = g_DiffuseTexture.Sample(LinearSampler, Get_UnknownUV(In)).r;
    if (fMask < 0.1f)
        discard;

    return PS_DIFF_SAMPLE(In, Get_BaseUV(In));
}

PS_OUT PS_GRASS_FUR(PS_IN In)
{
    return PS_DIFF_SAMPLE(In, Get_BaseUV(In));
}

PS_OUT PS_COLOR(PS_IN In)
{
    Apply_DitherIfNeeded(In.vPosition);

    float3 vMRA = g_MRATexture.Sample(LinearSampler, Get_MaterialUV(In)).rgb;
    float3 vNormal = Reconstruct_Normal(In, Get_NormalUV(In));

    return Make_GBufferOutput(
              In,
              g_vColor,
              vNormal,
              float4(vMRA, 1.f),
              float4(g_vEmissiveColor.rgb * g_vColor.a, 1.f));
}

PS_OUT PS_DISCARD(PS_IN In)
{
    discard;
    return (PS_OUT) 0;
}

PS_OUT PS_COLOR_MRA_DITHER(PS_IN In)
{
    Apply_Dissolve(In.vPosition);

    return Make_GBufferOutput(
                In,
                g_vColor,
                In.vNormal.xyz,
                float4(g_vMRA, 1.f),
                float4(g_vEmissiveColor.rgb, 1.f));
}

PS_OUT PS_COLOR_CONST_MRA(PS_IN In)
{
    Apply_DitherIfNeeded(In.vPosition);

    float3 vNormal = Reconstruct_Normal(In, Get_NormalUV(In));

    return Make_GBufferOutput(
                In,
                g_vColor,
                vNormal,
                float4(g_vMRA, 1.f),
                float4(g_vEmissiveColor.rgb * g_vColor.a, 1.f));
}



struct PS_DECAL_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vMRA : SV_TARGET2;
};

float2 Get_ScreenUV(float4 vProjPos)
{
    float2 vScreenUV;
    vScreenUV.x = vProjPos.x / vProjPos.w * 0.5f + 0.5f;
    vScreenUV.y = vProjPos.y / vProjPos.w * -0.5f + 0.5f;
    return vScreenUV;
}

float Sample_SceneDepth(float2 vScreenUV)
{
    float fSceneDepth = g_DepthTexture.Sample(PointSampler, vScreenUV).x;
    if (fSceneDepth <= 0.f)
        discard;

    return fSceneDepth;
}

float3 Recover_SceneWorldPosition(float2 vScreenUV, float fSceneDepth)
{
    return RecoverWorldPos(vScreenUV, fSceneDepth, g_ProjMatrixInverse, g_ViewMatrixInverse);
}

void Apply_DecalBoundsCut(float3 vLocalPosition)
{
    float3 vBoundsDistance = abs(vLocalPosition - g_vDecalBoundsCenter);
    if (any(vBoundsDistance > g_vDecalBoundsExtents + 0.0001f))
        discard;
}

void Apply_DecalMaterialMask(float2 vScreenPosition)
{
    uint iSurfaceMaterialID = g_MaterialIDTexture.Load(int3(vScreenPosition, 0));

    if ((0 == g_iDecalMaskMode && iSurfaceMaterialID == (uint) g_iDecalMaskID) ||
          (0 != g_iDecalMaskMode && iSurfaceMaterialID != (uint) g_iDecalMaskID))
    {
        discard;
    }
}

float2 Get_DecalUV(float3 vLocalPosition)
{
    return Apply_UVTransform(float2(vLocalPosition.x + 0.5f, 0.5f - vLocalPosition.z), g_vUVTransform);
}

float3 Decode_DecalNormal(float2 vUV, float3 vTangent, float3 vNormal, float3 vBinormal)
{
    float2 nrg = g_NormalTexture.Sample(LinearSampler, vUV).rg;
    nrg *= g_NormalStrength;

    float3 nT = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    float3 T = normalize(vTangent);
    float3 N = normalize(vNormal);
    float3 B = normalize(vBinormal);

    return normalize(nT.x * T + nT.y * B + nT.z * N);
}

PS_DECAL_OUT Make_DecalOutput(float2 vUV, float3 vTangent, float3 vNormal, float3 vBinormal)
{
    PS_DECAL_OUT Out = (PS_DECAL_OUT) 0;

    float4 vColor = g_DiffuseTexture.Sample(LinearSampler, vUV);
    if (vColor.a <= 0.f)
        discard;

    float fCoverage = vColor.a * g_fDecalAlpha;

    Out.vDiffuse = float4(vColor.rgb, fCoverage);

    [branch]
    if (0.f < g_fDecalHasNormal)
    {
        float3 vWorldNormal = Decode_DecalNormal(vUV, vTangent, vNormal, vBinormal);
        Out.vNormal = float4(vWorldNormal * 0.5f + 0.5f, fCoverage * g_fDecalHasNormal);
    }

    [branch]
    if (0.f < g_fDecalHasMRA)
    {
        float3 vMRA = g_MRATexture.Sample(LinearSampler, vUV).rgb;
        Out.vMRA = float4(vMRA, fCoverage * g_fDecalHasMRA);
    }

    return Out;
}