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

Texture2D g_TexDiff_Main;
Texture2D g_TexMRA_Main;
Texture2D g_TexNorm_Main;
Texture2D g_TexUkwn_Main;

Texture2D g_TexDiff_R;
Texture2D g_TexMRA_R;
Texture2D g_TexNorm_R;
Texture2D g_TexUkwn_R;

Texture2D g_TexDiff_G;
Texture2D g_TexMRA_G;
Texture2D g_TexNorm_G;
Texture2D g_TexUkwn_G;

Texture2D g_TexDiff_B;
Texture2D g_TexMRA_B;
Texture2D g_TexNorm_B;
Texture2D g_TexUkwn_B;

Texture2D g_TexDiff_A;
Texture2D g_TexMRA_A;
Texture2D g_TexNorm_A;
Texture2D g_TexUkwn_A;

uint g_iBase_UVIndex = 0;
uint g_iUnknown_UVIndex = 0;
uint g_iExtraR_UVIndex = 0;
uint g_iExtraG_UVIndex = 0;
uint g_iExtraB_UVIndex = 0;
uint g_iExtraA_UVIndex = 0;

uint g_iExtraR_MaskCh = 0;
uint g_iExtraG_MaskCh = 1;
uint g_iExtraB_MaskCh = 2;
uint g_iExtraA_MaskCh = 3;

float g_NormalStrength = 1.f;
float g_MaskStrength = 1.f;
float g_fAOStrength = 1.f;

float4 g_vUVTransform = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformNormal = float4(1.f, 1.f, 0.f, 0.f);
float4 g_vUVTransformMaterial = float4(1.f, 1.f, 0.f, 0.f);
float g_fUVRotate = 0.f;
float4 g_vExtraEnable = float4(0.f, 0.f, 0.f, 0.f);
float4 g_vEmissiveColor = float4(0.f, 0.f, 0.f, 0.f);
uint g_iMaterialID = 0;

uint g_bUseLayerEx = 0;

int4 g_iLayerExUVIndex[5];
float4 g_vLayerExEnable[5];

float4 g_vLayerExUVScale[20];
float4 g_vLayerExUVOffsetRotate[20];

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
    float4 vColor : COLOR0; // _c0  tint / blend
    float4 vColor1 : COLOR1; // _c1  AO / blend
    float4 vColor2 : COLOR2; // _c2  spare mask
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
    float4 vGeoNormal : SV_TARGET5;
    uint   vMaterialID : SV_TARGET6;
};

float2 Select_MapUV(PS_IN In, uint iUVIndex)
{
	[branch]
    if (iUVIndex == 1)
        return In.vTexcoord1;
	[branch]
    if (iUVIndex == 2)
        return In.vTexcoord2;
	[branch]
    if (iUVIndex == 3)
        return In.vTexcoord3;
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

float Select_VtxMask(PS_IN In, uint ch)
{
    float4 c = (ch < 4u) ? In.vColor : ((ch < 8u) ? In.vColor1 : In.vColor2);
    return c[ch & 3u];
}

void Blend_Extra(inout float3 albedo, PS_IN In, Texture2D tex, float enable, uint uvIdx, float mask)
{
    if (enable < 0.5f)
        return;
    float m = saturate((1.f - mask) * g_MaskStrength);
    float2 uv = Apply_MapUVTransform(Select_MapUV(In, uvIdx), g_vUVTransform);
    float3 o = tex.Sample(LinearSampler, uv).rgb;
    albedo = lerp(albedo, o, m);
}

float GetLayerExEnable(uint iGroup, uint iEntry)
{
    return g_vLayerExEnable[iGroup][iEntry];
}

float2 Apply_LayerExUVTransform(float2 uv, uint iFlat)
{
    uv *= g_vLayerExUVScale[iFlat].xy;

    float fRotate = g_vLayerExUVOffsetRotate[iFlat].z;
    float s = sin(fRotate);
    float c = cos(fRotate);

    uv = float2(
          uv.x * c - uv.y * s,
          uv.x * s + uv.y * c
      );

    uv += g_vLayerExUVOffsetRotate[iFlat].xy;
    return uv;
}

float2 GetLayerExUV(PS_IN In, uint iGroup, uint iEntry)
{
    uint iFlat = iGroup * 4u + iEntry;
    uint iUVIndex = (uint) g_iLayerExUVIndex[iGroup][iEntry];
    return Apply_LayerExUVTransform(Select_MapUV(In, iUVIndex), iFlat);
}

float4 SampleLayerExDiff(uint iGroup, float2 uv)
{
    [branch] if (iGroup == 0u) return g_TexDiff_Main.Sample(LinearSampler, uv);
    [branch] if (iGroup == 1u) return g_TexDiff_R.Sample(LinearSampler, uv);
    [branch] if (iGroup == 2u) return g_TexDiff_G.Sample(LinearSampler, uv);
    [branch] if (iGroup == 3u) return g_TexDiff_B.Sample(LinearSampler, uv);
    return g_TexDiff_A.Sample(LinearSampler, uv);
}

float4 SampleLayerExMRA(uint iGroup, float2 uv)
{
    [branch] if (iGroup == 0u) return g_TexMRA_Main.Sample(LinearSampler, uv);
    [branch] if (iGroup == 1u) return g_TexMRA_R.Sample(LinearSampler, uv);
    [branch] if (iGroup == 2u) return g_TexMRA_G.Sample(LinearSampler, uv);
    [branch] if (iGroup == 3u) return g_TexMRA_B.Sample(LinearSampler, uv);
    return g_TexMRA_A.Sample(LinearSampler, uv);
}

float4 SampleLayerExNorm(uint iGroup, float2 uv)
{
    [branch] if (iGroup == 0u) return g_TexNorm_Main.Sample(LinearSampler, uv);
    [branch] if (iGroup == 1u) return g_TexNorm_R.Sample(LinearSampler, uv);
    [branch] if (iGroup == 2u) return g_TexNorm_G.Sample(LinearSampler, uv);
    [branch] if (iGroup == 3u) return g_TexNorm_B.Sample(LinearSampler, uv);
    return g_TexNorm_A.Sample(LinearSampler, uv);
}

float4 SampleLayerExUkwn(uint iGroup, float2 uv)
{
    [branch] if (iGroup == 0u) return g_TexUkwn_Main.Sample(LinearSampler, uv);
    [branch] if (iGroup == 1u) return g_TexUkwn_R.Sample(LinearSampler, uv);
    [branch] if (iGroup == 2u) return g_TexUkwn_G.Sample(LinearSampler, uv);
    [branch] if (iGroup == 3u) return g_TexUkwn_B.Sample(LinearSampler, uv);
    return g_TexUkwn_A.Sample(LinearSampler, uv);
}

float GetLayerExBlendAmount(uint iGroup, uint iEntry, float mask)
{
    float enable = GetLayerExEnable(iGroup, iEntry);
    if (enable < 0.5f)
        return 0.f;

    return saturate((1.f - mask) * g_MaskStrength);
}

void Blend_LayerExDiff(inout float3 albedo, PS_IN In, uint iGroup, float mask)
{
    float m = GetLayerExBlendAmount(iGroup, 0u, mask);
    if (m <= 0.f)
        return;

    float2 uv = GetLayerExUV(In, iGroup, 0u);
    float3 o = SampleLayerExDiff(iGroup, uv).rgb;
    albedo = lerp(albedo, o, m);
}

void Blend_LayerExMRA(inout float3 mra, PS_IN In, uint iGroup, float mask)
{
    float m = GetLayerExBlendAmount(iGroup, 1u, mask);
    if (m <= 0.f)
        return;

    float2 uv = GetLayerExUV(In, iGroup, 1u);
    float3 o = SampleLayerExMRA(iGroup, uv).rgb;
    mra = lerp(mra, o, m);
}

void Blend_LayerExNormRG(inout float2 nrg, PS_IN In, uint iGroup, float mask)
{
    float m = GetLayerExBlendAmount(iGroup, 2u, mask);
    if (m <= 0.f)
        return;

    float2 uv = GetLayerExUV(In, iGroup, 2u);
    float2 o = SampleLayerExNorm(iGroup, uv).rg;
    nrg = lerp(nrg, o, m);
}

void Blend_LayerExUkwn(inout float3 albedo, PS_IN In, uint iGroup, float mask)
{
    float m = GetLayerExBlendAmount(iGroup, 3u, mask);
    if (m <= 0.f)
        return;

    float2 uv = GetLayerExUV(In, iGroup, 3u);
    float3 o = SampleLayerExUkwn(iGroup, uv).rgb;
    albedo = lerp(albedo, o, m);
}

void Blend_LayerExMaterial(inout float3 albedo, inout float3 mra, inout float2 nrg, PS_IN In, uint iGroup, float mask)
{
    Blend_LayerExDiff(albedo, In, iGroup, mask);
    Blend_LayerExMRA(mra, In, iGroup, mask);
    Blend_LayerExNormRG(nrg, In, iGroup, mask);
}

PS_OUT PS_DMN_LayerEx(PS_IN In)
{
    PS_OUT Out;

    float2 vBaseUV = GetLayerExUV(In, 0u, 0u);
    float2 vMaterialUV = GetLayerExUV(In, 0u, 1u);
    float2 vNormalUV = GetLayerExUV(In, 0u, 2u);

    float4 vBase = SampleLayerExDiff(0u, vBaseUV);
    if (vBase.a < 0.1f)
        discard;

    float3 albedo = vBase.rgb;
    float3 mra = SampleLayerExMRA(0u, vMaterialUV).rgb;

    float3 N = normalize(In.vNormal.xyz);
    float3 T = normalize(In.vTangent.xyz);
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * In.vTangent.w;
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = SampleLayerExNorm(0u, vNormalUV).rg;
    nrg *= g_NormalStrength;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    float3 Nw = normalize(mul(nTS, TBN));

    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_WHITE(PS_IN In)
{
    PS_OUT Out;
    Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
    Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f); // 노멀맵 없음 → 기하노멀
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, 1.f); // metal0 / rough1 / ao1 기본
    Out.vEmissive = float4(g_vEmissiveColor.rgb, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
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

    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
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

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg;
    nrg *= g_NormalStrength;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    float3 Nw = normalize(mul(nTS, TBN));

    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_DMN(PS_IN In)
{
    if (g_bUseLayerEx != 0u)
        return PS_DMN_LayerEx(In);
    
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

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg;
    nrg *= g_NormalStrength;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    float3 Nw = normalize(mul(nTS, TBN));

    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_DMNU(PS_IN In)
{
    PS_OUT Out = PS_DMN(In);
    float2 vUnknownUV = Apply_MapUVTransform(Select_MapUV(In, g_iUnknown_UVIndex), g_vUVTransform);
    float4 vUnknown = g_UnknownTexture.Sample(LinearSampler, vUnknownUV);
    return Out;
}

PS_OUT PS_DMN_TOP(PS_IN In)
{
    PS_OUT Out;

    float3 wp = In.vWorldPos.xyz;
    float2 vBaseUV = Apply_MapUVTransform(wp.xz, g_vUVTransform);
    float2 vNormalUV = Apply_MapUVTransform(wp.xz, g_vUVTransformNormal);
    float2 vMaterialUV = Apply_MapUVTransform(wp.xz, g_vUVTransformMaterial);

    float4 vBase = g_DiffuseTexture.Sample(LinearSampler, vBaseUV);
    if (vBase.a < 0.1f)
        discard;

    float3 albedo = vBase.rgb;
    float3 mra = g_MRATexture.Sample(LinearSampler, vMaterialUV).rgb;

    float3 N = normalize(In.vNormal.xyz);
    float3 T = normalize(cross(float3(0.f, 0.f, 1.f), N));
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);

    float2 nrg = g_NormalTexture.Sample(LinearSampler, vNormalUV).rg;
    nrg *= g_NormalStrength;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    float3 Nw = normalize(mul(nTS, TBN));

    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_DMN_MASK_LayerEx(PS_IN In)
{
    PS_OUT Out;

    float2 vBaseUV = GetLayerExUV(In, 0u, 0u);
    float2 vMaterialUV = GetLayerExUV(In, 0u, 1u);
    float2 vNormalUV = GetLayerExUV(In, 0u, 2u);

    float4 vBase = SampleLayerExDiff(0u, vBaseUV);
    if (vBase.a < 0.1f)
        discard;

    float3 albedo = vBase.rgb;
    float3 mra = SampleLayerExMRA(0u, vMaterialUV).rgb;
    float2 nrg = SampleLayerExNorm(0u, vNormalUV).rg;

    Blend_LayerExMaterial(albedo, mra, nrg, In, 1u, In.vColor.r);
    Blend_LayerExMaterial(albedo, mra, nrg, In, 2u, In.vColor.g);
    Blend_LayerExMaterial(albedo, mra, nrg, In, 3u, In.vColor.b);
    Blend_LayerExMaterial(albedo, mra, nrg, In, 4u, In.vColor.a);

    float3 N = normalize(In.vNormal.xyz);
    float3 T = normalize(In.vTangent.xyz);
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * In.vTangent.w;
    float3x3 TBN = float3x3(T, B, N);

    nrg *= g_NormalStrength;
    float3 nTS = float3(nrg, sqrt(saturate(1.f - dot(nrg, nrg))));
    float3 Nw = normalize(mul(nTS, TBN));

    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(Nw * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(mra, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_DMN_MASK(PS_IN In)
{
    if (g_bUseLayerEx != 0u)
        return PS_DMN_MASK_LayerEx(In);

    PS_OUT Out = PS_DMN(In);
    float3 albedo = Out.vDiffuse.rgb;

    Blend_Extra(albedo, In, g_ExtraRTexture, g_vExtraEnable.x, g_iExtraR_UVIndex, In.vColor.r);
    Blend_Extra(albedo, In, g_ExtraGTexture, g_vExtraEnable.y, g_iExtraG_UVIndex, In.vColor.g);
    Blend_Extra(albedo, In, g_ExtraBTexture, g_vExtraEnable.z, g_iExtraB_UVIndex, In.vColor.b);
    Blend_Extra(albedo, In, g_ExtraATexture, g_vExtraEnable.w, g_iExtraA_UVIndex, In.vColor.a);

    Out.vDiffuse.rgb = albedo;
    return Out;
}

PS_OUT PS_UKWN_LayerEx(PS_IN In)
{
    PS_OUT Out;
    float2 vBaseUV = GetLayerExUV(In, 0u, 3u);
    float4 vBase = SampleLayerExUkwn(0u, vBaseUV);
    if (vBase.a < 0.1f)
        discard;

    float3 albedo = vBase.rgb;
    
    Blend_LayerExUkwn(albedo, In, 1u, In.vColor.r);
    Blend_LayerExUkwn(albedo, In, 2u, In.vColor.g);
    Blend_LayerExUkwn(albedo, In, 3u, In.vColor.b);
    Blend_LayerExUkwn(albedo, In, 4u, In.vColor.a);

    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_UKWN(PS_IN In)
{
    if (g_bUseLayerEx != 0u)
        return PS_UKWN_LayerEx(In);
    
    PS_OUT Out;
    float2 vBaseUV = Apply_MapUVTransform(Select_MapUV(In, g_iBase_UVIndex), g_vUVTransform);
    float4 vBase = g_UnknownTexture.Sample(LinearSampler, vBaseUV);
    if (vBase.a < 0.1f)
        discard;

    float3 albedo = vBase.rgb;
    
    Out.vDiffuse = float4(albedo, 1.f);
    Out.vNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 0.f);
    Out.vMRA = float4(0.f, 1.f, 1.f, 1.f);
    Out.vEmissive = float4(g_vEmissiveColor.rgb * vBase.a, 1.f);
    Out.vGeoNormal = float4(normalize(In.vNormal.xyz) * 0.5f + 0.5f, 0.f);
    Out.vMaterialID = g_iMaterialID;
    return Out;
}

PS_OUT PS_DISCARD(PS_IN In)
{
    discard;
    return (PS_OUT) 0;
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
    pass White // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_WHITE();
    }
    pass DIFF // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DIFF();
    }
    pass DN // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DN();
    }
    pass DMN // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DMN();
    }
    pass DMNU // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DMNU();
    }
    pass TOP // 6  
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DMN_TOP();
    }

    pass MASK // 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DMN_MASK();
    }
    pass DISCARD // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISCARD();
    }
}
