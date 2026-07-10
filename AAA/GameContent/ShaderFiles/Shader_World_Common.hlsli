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

#define WORLD_FLAG_DITHER 0x01u

uint g_iWorldFlags = 0u;
float g_fDissolve = 0.f;

uint g_iMaterialID = 0u;
uint g_iShadowAlphaSource = 0u;

#define WORLD_SHADOW_ALPHA_NONE 0u
#define WORLD_SHADOW_ALPHA_DIFFUSE 1u
#define WORLD_SHADOW_ALPHA_UNKNOWN 2u
#define WORLD_SHADOW_ALPHA_DISCARD_ALL 3u
#define WORLD_SHADOW_ALPHA_DIFFUSE_R 4u
#define WORLD_SHADOW_ALPHA_UNKNOWN_R 5u

Texture2D g_DepthTexture;
Texture2D<uint> g_MaterialIDTexture;

float3 g_vDecalBoundsCenter = float3(0.f, 0.f, 0.f);
float3 g_vDecalBoundsExtents = float3(0.5f, 0.5f, 0.5f);
float g_fDecalAlpha = 1.f;

float g_fDecalHasNormal = 0.f;
float g_fDecalHasMRA = 0.f;

int g_iDecalMaskMode = 0;
int g_iDecalMaskID = 200;

struct WORLD_PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;

    float2 vTexcoord0 : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;

    float4 vWorldPos : TEXCOORD4;
    float4 vProjPos : TEXCOORD5;

    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

struct WORLD_PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vMRA : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
    float4 vGeoNormal : SV_TARGET5;
    uint vMaterialID : SV_TARGET6;
};