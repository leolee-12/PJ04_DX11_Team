#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ViewMatrixInverse;
float4x4 g_ProjMatrix;
float4x4 g_ProjMatrixInverse;

Texture2D g_WaterNormalTexture1;
Texture2D g_WaterNormalTexture2;
Texture2D g_WaterNoiseTexture;
Texture2D g_WaterCausticTexture;
Texture2D g_SceneTexture;
Texture2D g_DepthTexture;
TextureCube g_IrradianceCube;
TextureCube g_PrefilteredCube;

float4 g_vCamPosition;

float4 g_vLightDir = float4(0.f, -1.f, 0.f, 0.f);
float4 g_vLightSpecular = float4(1.f, 1.f, 1.f, 1.f);

uint g_iSpecularMip = 1u;
float g_fIBLIntensity = 1.f;

float4 g_vShallowColor = float4(0.15f, 0.55f, 0.72f, 1.f);
float4 g_vDeepColor = float4(0.02f, 0.16f, 0.30f, 1.f);
float g_fShallowColorStrength = 0.45f;
float g_fOpacity = 0.65f;
float g_fDepthFadeDistance = 5.f;

float2 g_vNormalTiling0 = float2(0.08f, 0.08f);
float2 g_vNormalSpeed0 = float2(0.02f, 0.01f);
float2 g_vNormalTiling1 = float2(0.14f, 0.14f);
float2 g_vNormalSpeed1 = float2(-0.012f, 0.018f);
float g_fNormalStrength = 1.f;

float g_fFresnelPower = 5.f;
float g_fReflectionStrength = 0.8f;
float g_fRefractionStrength = 0.015f;

float g_fLightReceiveStrength = 1.f;
float g_fSpecularPower = 64.f;
float g_fSpecularStrength = 1.f;

float g_fFoamWidth = 0.4f;
float g_fFoamStrength = 0.7f;
float2 g_vFoamNoiseTiling = float2(0.35f, 0.35f);
float2 g_vFoamNoiseSpeed = float2(0.03f, -0.02f);
float g_fFoamNoiseStrength = 0.6f;
float g_fFoamBlur = 0.f;

float2 g_vCausticTiling = float2(0.05f, 0.05f);
float2 g_vCausticSpeed = float2(0.015f, 0.012f);
float g_fCausticStrength = 0.5f;
float g_fCausticNoiseStrength = 0.6f;
float g_fCausticBlur = 0.f;

float g_fVisibility = 1.f;
float g_fGameTime = 0.f;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord0 : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    float2 vTexcoord3 : TEXCOORD3;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vProjPosition : TEXCOORD0;
    float3 vWorldPosition : TEXCOORD1;
    float3 vViewPosition : TEXCOORD2;
    float3 vWorldNormal : TEXCOORD3;
    float3 vWorldTangent : TEXCOORD4;
    float3 vWorldBinormal : TEXCOORD5;
};

PS_IN VS_MAIN(VS_IN In)
{
    PS_IN Out = (PS_IN) 0;

    float4 vWorldPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    float4 vViewPosition = mul(vWorldPosition, g_ViewMatrix);
    float4 vProjPosition = mul(vViewPosition, g_ProjMatrix);

    Out.vPosition = vProjPosition;
    Out.vProjPosition = vProjPosition;
    Out.vWorldPosition = vWorldPosition.xyz;
    Out.vViewPosition = vViewPosition.xyz;
    Out.vWorldNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix).xyz);
    Out.vWorldTangent = normalize(mul(float4(In.vTangent.xyz, 0.f), g_WorldMatrix).xyz);
    Out.vWorldBinormal = normalize(mul(float4(In.vBinormal.xyz, 0.f), g_WorldMatrix).xyz);

    return Out;
}

float2 Get_ScreenUV(float4 vProjPosition)
{
    float2 vScreenUV;
    vScreenUV.x = vProjPosition.x / vProjPosition.w * 0.5f + 0.5f;
    vScreenUV.y = vProjPosition.y / vProjPosition.w * -0.5f + 0.5f;
    return saturate(vScreenUV);
}

float Recover_ViewDepth(float2 vScreenUV, float fDepth)
{
    float4 vViewPosition;
    vViewPosition.x = vScreenUV.x * 2.f - 1.f;
    vViewPosition.y = vScreenUV.y * -2.f + 1.f;
    vViewPosition.z = fDepth;
    vViewPosition.w = 1.f;
    vViewPosition = mul(vViewPosition, g_ProjMatrixInverse);

    return vViewPosition.z / vViewPosition.w;
}

float3 Decode_BC5_SNORM(float2 vNormalRG)
{
    float fNormalZ = sqrt(saturate(1.f - dot(vNormalRG, vNormalRG)));
    return normalize(float3(vNormalRG, fNormalZ));
}

float3 Build_WaterNormal(PS_IN In, float fFaceSign)
{
    float3 vGeometryNormal = normalize(In.vWorldNormal) * fFaceSign;
    float3 vTangent = normalize(In.vWorldTangent);
    float3 vBinormal = normalize(In.vWorldBinormal) * fFaceSign;

    float2 vNormalUV0 = In.vWorldPosition.xz * g_vNormalTiling0 + g_fGameTime * g_vNormalSpeed0;
    float2 vNormalUV1 = In.vWorldPosition.xz * g_vNormalTiling1 + g_fGameTime * g_vNormalSpeed1;

    float2 vNormalRG0 = g_WaterNormalTexture1.Sample(LinearSampler, vNormalUV0).rg;
    float2 vNormalRG1 = g_WaterNormalTexture2.Sample(LinearSampler, vNormalUV1).rg;
    float2 vCombinedNormalRG = (vNormalRG0 + vNormalRG1) * 0.5f * g_fNormalStrength;

    float3 vTangentNormal = Decode_BC5_SNORM(vCombinedNormalRG);

    return normalize(
                vTangentNormal.x * vTangent
                + vTangentNormal.y * vBinormal
                + vTangentNormal.z * vGeometryNormal);
}

float4 PS_MAIN(PS_IN In, bool bIsFrontFace : SV_IsFrontFace) : SV_TARGET0
{
    float fFaceSign = bIsFrontFace ? 1.f : -1.f;
    float3 vGeometryNormal = normalize(In.vWorldNormal) * fFaceSign;
    float3 vWaterNormal = Build_WaterNormal(In, fFaceSign);
    float3 vViewDirection = normalize(g_vCamPosition.xyz - In.vWorldPosition);

    float2 vScreenUV = Get_ScreenUV(In.vProjPosition);
    float fSceneDepth = g_DepthTexture.Sample(PointSampler, vScreenUV).r;

    float fDepthDifference = max(g_fDepthFadeDistance, 0.001f);

    if (fSceneDepth > 0.f)
    {
        float fSceneViewDepth = Recover_ViewDepth(vScreenUV, fSceneDepth);
        fDepthDifference = max(fSceneViewDepth - In.vViewPosition.z, 0.f);
    }

    // IBL 적용
    float fDepthRatio = saturate(fDepthDifference / max(g_fDepthFadeDistance, 0.001f));
    float3 vWaterAlbedo = saturate(lerp(g_vShallowColor.rgb, g_vDeepColor.rgb, fDepthRatio));
    float3 vIrradiance = g_IrradianceCube.Sample(LinearSampler, vGeometryNormal).rgb;
    float3 vLitWaterColor = saturate(vWaterAlbedo * vIrradiance * g_fIBLIntensity);
    float3 vWaterColor = lerp(vWaterAlbedo, vLitWaterColor, saturate(g_fLightReceiveStrength));

    // 굴절
    float3 vViewNormalDelta = mul(float4(vWaterNormal - vGeometryNormal, 0.f), g_ViewMatrix).xyz;
    float2 vRefractionOffset = float2(vViewNormalDelta.x, -vViewNormalDelta.y) * g_fRefractionStrength;
    float2 vRefractionUV = saturate(vScreenUV + vRefractionOffset);

    float fRefractionDepth = g_DepthTexture.Sample(PointSampler, vRefractionUV).r;
    if (fRefractionDepth > 0.f && Recover_ViewDepth(vRefractionUV, fRefractionDepth) < In.vViewPosition.z)
    {
        vRefractionUV = vScreenUV;
        fRefractionDepth = fSceneDepth;
    }

    float3 vRefractedColor = g_SceneTexture.Sample(ClampSampler, vRefractionUV).rgb;

    float fFresnel = pow(1.f - saturate(dot(vWaterNormal, vViewDirection)), g_fFresnelPower);
    float3 vReflectionDirection = reflect(-vViewDirection, vWaterNormal);

    float fRoughness = saturate(sqrt(2.f / (g_fSpecularPower + 2.f)));
    float fReflectionMip = fRoughness * max((float) g_iSpecularMip - 1.f, 0.f);
    float3 vReflectionColor = g_PrefilteredCube.SampleLevel(ClampSampler, vReflectionDirection, fReflectionMip).rgb;
    vReflectionColor *= g_fIBLIntensity;

    float3 vLightDirection = normalize(-g_vLightDir.xyz);
    float3 vHalfDirection = normalize(vViewDirection + vLightDirection);
    float fSpecular = pow(saturate(dot(vWaterNormal, vHalfDirection)), g_fSpecularPower);
    fSpecular *= saturate(dot(vWaterNormal, vLightDirection));
    fSpecular *= g_fSpecularStrength;

    // Caustic
    if (fRefractionDepth > 0.f)
    {
        float3 vFloorPosition = RecoverWorldPos(
                vRefractionUV, fRefractionDepth, g_ProjMatrixInverse, g_ViewMatrixInverse);
        float2 vCausticUV = vFloorPosition.xz * g_vCausticTiling + g_fGameTime * g_vCausticSpeed;

        float fCaustic = min(
                g_WaterCausticTexture.SampleBias(LinearSampler, vCausticUV, g_fCausticBlur).r,
                g_WaterCausticTexture.SampleBias(LinearSampler, vCausticUV * 0.73f + 0.37f, g_fCausticBlur).r);

        float2 vCausticNoiseUV = vFloorPosition.xz * g_vFoamNoiseTiling + g_fGameTime * g_vFoamNoiseSpeed;
        float fCausticNoise = g_WaterNoiseTexture.SampleBias(LinearSampler, vCausticNoiseUV, g_fCausticBlur).r;
        fCaustic *= lerp(1.f, fCausticNoise, saturate(g_fCausticNoiseStrength));

        vRefractedColor += g_vLightSpecular.rgb * fCaustic * g_fCausticStrength;
    }

    // Water Color 합성
    float fWaterColorWeight = lerp(saturate(g_fShallowColorStrength), 1.f, fDepthRatio) * saturate(g_fOpacity);
    float3 vBaseColor = lerp(vRefractedColor, vWaterColor, fWaterColorWeight);
    float3 vFinalColor = vBaseColor;
    vFinalColor += vReflectionColor * fFresnel * g_fReflectionStrength;
    vFinalColor += g_vLightSpecular.rgb * fSpecular;

    // Foam
    float2 vFoamUV = In.vWorldPosition.xz * g_vFoamNoiseTiling + g_fGameTime * g_vFoamNoiseSpeed;
    float fFoamNoise = g_WaterNoiseTexture.SampleBias(LinearSampler, vFoamUV, g_fFoamBlur).r;
    float fFoamEdge = max(g_fFoamWidth, 0.001f);
    float fFoam = 1.f - smoothstep(0.f, fFoamEdge,
            fDepthDifference + (fFoamNoise - 0.5f) * fFoamEdge * g_fFoamNoiseStrength);
    fFoam = saturate(fFoam * g_fFoamStrength);
    vFinalColor = lerp(vFinalColor, float3(1.f, 1.f, 1.f), fFoam);

    float fFinalAlpha = saturate(lerp(g_fOpacity * 0.65f, g_fOpacity, fDepthRatio));
    fFinalAlpha *= saturate(g_fVisibility);

    return float4(max(vFinalColor, 0.f), 1.f);
}

technique11 DefaultTechnique
{
    pass SURFACE
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}