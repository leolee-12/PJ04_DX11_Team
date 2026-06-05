#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_SceneTexture; // HDR 씬
Texture2D g_BloomTexture; // 블룸 결과
float2 g_vBlurDir; // (1,0)=H, (0,1)=V
float2 g_vTexelSize; // 1/해상도 (블룸 타겟 기준)

// Bloom Global
float g_fThreshold = 1.0f;
float g_fBloomIntensity = 1.0f;

/* SSAO 입력 (G-buffer) */
Texture2D g_DepthTexture; // x=z/w, y=viewZ/500
Texture2D g_NormalTexture; // 월드노멀*0.5+0.5
Texture2D g_SSAOTexture; // SSAO 결과 (블러 입력)

float4x4 g_ProjMatrixInverse; // 카메라 역투영 (뷰공간 재구성)
float4x4 g_CamViewMatrix; // 카메라 뷰 (월드노멀 → 뷰공간)
float4x4 g_CamProjMatrix; // 카메라 투영 (뷰공간 샘플 → UV)

// SSAO Global
float g_fSSAORadius = 5.0f; // ★씬 스케일 따라 튜닝 (viewZ~500 스케일이라 작으면 안 보임)
float g_fSSAOBias = 0.015f; // viewZ 에 곱해지는 비례계수
float g_fSSAOPower = 1.8f;

/* SSR 입력 */
Texture2D g_MRATexture; // r=metallic, g=roughness, b=ao
Texture2D g_DiffuseTexture; // albedo (F0)
float g_fSSRIntensity = 1.0f;
float g_fSSRMaxDistance = 30.0f;
float g_fSSRThickness = 0.5f;
static const int SSR_STEPS = 64;

/* SSR 폴백용 IBL */
TextureCube g_PrefilteredCube; // 디퍼드와 동일 큐브
int g_iSpecularMip = 1;
float g_fIBLIntensity = 1.f;
float4x4 g_CamViewMatrixInverse;

    //============================ Common VS ============================
struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};
struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    float4 vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vPosition = mul(vPosition, g_ViewMatrix);
    vPosition = mul(vPosition, g_ProjMatrix);
    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

// === SSAO 헬퍼 ==== //
float3 ViewPosFromUV(float2 uv, float ndcZ)
{
    float4 p = float4(uv.x * 2.f - 1.f, uv.y * -2.f + 1.f, ndcZ, 1.f);
    p = mul(p, g_ProjMatrixInverse); // 뷰공간
    return p.xyz / p.w;
}
float2 EnvBRDFApprox(float rough, float NdotV)
{
    const float4 c0 = float4(-1.f, -0.0275f, -0.572f, 0.022f);
    const float4 c1 = float4(1.f, 0.0425f, 1.04f, -0.04f);
    float4 r = rough * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28f * NdotV)) * r.x + r.y;
    return float2(-1.04f, 1.04f) * a004 + r.zw;
}
float3 Hash3(float2 uv, float i)
{
    float3 q = float3(dot(uv, float2(127.1f, 311.7f)),
                        dot(uv, float2(269.5f, 183.3f)),
                        dot(uv, float2(419.2f, 371.9f)));
    q += i * float3(13.13f, 17.17f, 19.19f);
    return frac(sin(q) * 43758.5453f) * 2.f - 1.f;
}

  //============================ Bloom Bright (pass 0) ============================
float4 PS_BLOOMBRIGHT(PS_IN In) : SV_TARGET0
{
    float3 c = g_SceneTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float luma = dot(c, float3(0.2126f, 0.7152f, 0.0722f));
    float k = max(luma - g_fThreshold, 0.f) / max(luma, 1e-4f); // soft knee
    return float4(c * k, 1.f);
}

  //============================ Blur (pass 1) ============================
float4 PS_BLUR(PS_IN In) : SV_TARGET0
{
    const float w[5] = { 0.227027f, 0.194594f, 0.121622f, 0.054054f, 0.016216f };
    float3 result = g_BloomTexture.Sample(LinearSampler, In.vTexcoord).rgb * w[0];
    
    [unroll]
    for (int i = 1; i < 5; ++i)
    {
        float2 off = g_vBlurDir * g_vTexelSize * i;
        result += g_BloomTexture.Sample(LinearSampler, In.vTexcoord + off).rgb * w[i];
        result += g_BloomTexture.Sample(LinearSampler, In.vTexcoord - off).rgb * w[i];
    }
    return float4(result, 1.f);
}

  //============================ Composite (pass 2) ============================
float4 PS_COMPOSITE(PS_IN In) : SV_TARGET0
{
    float4 scene = g_SceneTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == scene.a)
        discard;

    float4 bloom = g_BloomTexture.Sample(LinearSampler, In.vTexcoord);

    float3 color = scene.rgb + bloom.rgb * g_fBloomIntensity;
    color = color / (color + 1.f); // Reinhard
    color = pow(color, 1.f / 2.2f); // 감마
    return float4(color, 1.f);
}

  //============================ SSAO (pass 3) ============================
float4 PS_SSAO(PS_IN In) : SV_TARGET0
{
    float ndcZ = g_DepthTexture.Sample(PointSampler, In.vTexcoord).x;
    if (ndcZ == 0.f)              // 배경 → 차폐 없음
        return 1.f.xxxx;

    float3 viewPos = ViewPosFromUV(In.vTexcoord, ndcZ);
    float3 worldN = normalize(g_NormalTexture.Sample(PointSampler, In.vTexcoord).xyz * 2.f - 1.f);
    float3 viewN = normalize(mul(worldN, (float3x3) g_CamViewMatrix));

    const int KERNEL = 16;
    float occlusion = 0.f;

      [unroll]
    for (int i = 0; i < KERNEL; ++i)
    {
        float3 dir = Hash3(In.vTexcoord, (float) i);
        if (dot(dir, viewN) < 0.f)
            dir = -dir; // 반구로
        float scale = (float) i / KERNEL;
        scale = lerp(0.1f, 1.f, scale * scale); // 가까운 샘플 밀집
        float3 samplePos = viewPos + dir * scale * g_fSSAORadius;

        float4 offset = mul(float4(samplePos, 1.f), g_CamProjMatrix);
        offset.xyz /= offset.w;
        float2 sUV = offset.xy * float2(0.5f, -0.5f) + 0.5f;
        if (sUV.x < 0.f || sUV.x > 1.f || sUV.y < 0.f || sUV.y > 1.f)
            continue;

        float sceneZ = ViewPosFromUV(sUV, g_DepthTexture.Sample(PointSampler, sUV).x).z;
        float rangeCheck = smoothstep(0.f, 1.f, g_fSSAORadius / max(abs(viewPos.z - sceneZ), 1e-4f));
        
        float bias = g_fSSAOBias * g_fSSAORadius;
        occlusion += (sceneZ < samplePos.z - bias ? 1.f : 0.f) * rangeCheck;
    }

    float ao = pow(saturate(1.f - occlusion / KERNEL), g_fSSAOPower);
    return ao.xxxx;
}

    //============================ SSAO Blur (pass 4) ============================
float4 PS_SSAO_BLUR(PS_IN In) : SV_TARGET0
{
    float result = 0.f;
      [unroll]
    for (int x = -2; x <= 2; ++x)
      [unroll]
        for (int y = -2; y <= 2; ++y)
            result += g_SSAOTexture.Sample(PointSampler, In.vTexcoord + float2(x, y) * g_vTexelSize).r;
    return (result / 25.f).xxxx; // 5x5 박스 블러로 해시 노이즈 제거
}

   //============================ SSR (pass 5) ============================
float4 PS_SSR(PS_IN In) : SV_TARGET0
{
    float4 scene = g_SceneTexture.Sample(LinearSampler, In.vTexcoord);

    float ndcZ = g_DepthTexture.Sample(PointSampler, In.vTexcoord).x;
    if (ndcZ == 0.f)
        return scene; // 배경

    float4 mra4 = g_MRATexture.Sample(PointSampler, In.vTexcoord);
    float3 mra = mra4.rgb;
    float metallic = mra.r;
    float roughness = mra.g;
    float ao = mra.b;
    
    uint matID = (uint) round(mra4.a * 255.f);
    bool bWater = (matID == MAT_WATER);

    float3 albedo = g_DiffuseTexture.Sample(PointSampler, In.vTexcoord).rgb;
    float3 worldN = normalize(g_NormalTexture.Sample(PointSampler, In.vTexcoord).xyz * 2.f - 1.f);

    float3 viewPos = ViewPosFromUV(In.vTexcoord, ndcZ);
    float3 viewN = normalize(mul(worldN, (float3x3) g_CamViewMatrix));
    float3 viewDir = normalize(viewPos);
    float NdotV = saturate(dot(viewN, -viewDir));

    float3 F0 = lerp(0.04f, albedo, metallic);

      /* --- IBL 스펙큘러 (폴백 + 거친 표면 기본 반사) --- */
    float3 R_view = reflect(viewDir, viewN);
    float3 R_world = normalize(mul(float4(R_view, 0.f), g_CamViewMatrixInverse).xyz);
    float3 prefiltered = g_PrefilteredCube.SampleLevel(LinearSampler, R_world, roughness * (g_iSpecularMip - 1)).rgb;
    float2 envBRDF = EnvBRDFApprox(roughness, NdotV);
    float ssao = g_SSAOTexture.Sample(PointSampler, In.vTexcoord).r;
    float3 iblSpec = prefiltered * (F0 * envBRDF.x + envBRDF.y) * ao * ssao * g_fIBLIntensity;

      /* --- 화면공간 반사 트레이스 (매끈한 표면만) --- */
    float conf = 0.f;
    float3 ssrColor = 0.f.xxx;

    if (roughness <= 0.8f)
    {
        float3 rayStart = viewPos + viewN * 0.05f;
        float3 rayEnd = rayStart + R_view * g_fSSRMaxDistance;

        const float zNear = 0.1f;
        if (rayEnd.z < zNear)
        {
            float tc = (zNear - rayStart.z) / (rayEnd.z - rayStart.z);
            rayEnd = rayStart + (rayEnd - rayStart) * tc;
        }

        float4 H0 = mul(float4(rayStart, 1.f), g_CamProjMatrix);
        float4 H1 = mul(float4(rayEnd, 1.f), g_CamProjMatrix);
        float k0 = 1.f / H0.w, k1 = 1.f / H1.w;
        float2 P0 = (H0.xy * k0) * float2(0.5f, -0.5f) + 0.5f;
        float2 P1 = (H1.xy * k1) * float2(0.5f, -0.5f) + 0.5f;
        float Q0 = rayStart.z * k0, Q1 = rayEnd.z * k1;

        float hit = 0.f;
        float2 hitUV = 0.f;
        float prevT = 0.f;

            [loop]
        for (int i = 1; i <= SSR_STEPS; ++i)
        {
            float t = (float) i / SSR_STEPS;
            float2 uv = lerp(P0, P1, t);
            if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
                break;

            float rayZ = lerp(Q0, Q1, t) / lerp(k0, k1, t);
            float sNdc = g_DepthTexture.Sample(PointSampler, uv).x;
            if (sNdc == 0.f)
            {
                prevT = t;
                continue;
            }
            float sceneZ = ViewPosFromUV(uv, sNdc).z;

            float diff = rayZ - sceneZ;
            if (diff > 0.f && diff < g_fSSRThickness)
            {
                float a = prevT, b = t;
                    [unroll]
                for (int j = 0; j < 5; ++j)
                {
                    float m = (a + b) * 0.5f;
                    float2 muv = lerp(P0, P1, m);
                    float mRayZ = lerp(Q0, Q1, m) / lerp(k0, k1, m);
                    float mSceneZ = ViewPosFromUV(muv, g_DepthTexture.Sample(PointSampler, muv).x).z;
                    if (mRayZ - mSceneZ > 0.f)
                        b = m;
                    else
                        a = m;
                }
                hitUV = lerp(P0, P1, b);
                hit = 1.f;
                break;
            }
            prevT = t;
        }

        if (hit > 0.5f)
        {
            float3 F = F0 + (max((1.f - roughness).xxx, F0) - F0) * pow(1.f - NdotV, 5.f);
            float3 hitColor = g_SceneTexture.Sample(LinearSampler, hitUV).rgb;
            ssrColor = hitColor * F;

            float roughFade = saturate(1.f - roughness / 0.8f);
            float2 e = smoothstep(0.f, 0.15f, hitUV) * smoothstep(0.f, 0.15f, 1.f - hitUV);
            float edgeFade = e.x * e.y;
            conf = saturate(hit * roughFade * edgeFade * g_fSSRIntensity);
            conf *= smoothstep(0.02f, 0.10f, roughness);
        }
    }

      /* --- 단일 합성: SSR(맞으면) ↔ 큐브맵(폴백) --- */
    float3 reflection = lerp(iblSpec, ssrColor, conf);
    return float4(scene.rgb + reflection, scene.a);
}

    //============================ Technique ============================
technique11 DefaultTechnique
{
    pass BloomBright // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLOOMBRIGHT();
    }
    pass Blur // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR();
    }
    pass Composite // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_COMPOSITE();
    }
    pass SSAO // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSAO();
    }
    pass SSAO_Blur // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSAO_BLUR();
    }
    pass SSR // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSR();
    }
}