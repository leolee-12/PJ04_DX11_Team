#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_SceneTexture; // HDR 씬
Texture2D g_BloomTexture; // 블룸 결과
float2 g_vBlurDir; // (1,0)=H, (0,1)=V
float2 g_vTexelSize; // 1/해상도 (블룸 타겟 기준)
Texture2D g_LightDepthTexture; // Target_LightDepth (raw depth .r)
float g_fESMConst = 80.f;

// Bloom Global
float g_fThreshold = 1.0f;
float g_fBloomIntensity = 1.0f;

/* SSAO 입력 (G-buffer) */
Texture2D g_DepthTexture; // x=z/w, y=viewZ/500
Texture2D g_NormalTexture; // 월드노멀*0.5+0.5
Texture2D g_GeoNormalTexture;
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

//DoF
float g_fDoFEnable = 0.f;
float g_fFocusDist = 12.f; // AutoFocus=0일 때 수동 초점거리(view 단위)
float g_fAperture = 1.5f; // 조리개: 클수록 얕은 심도(블러↑, near 더 빨리)
float g_fDoFMaxCoC = 12.f; // 최대 blur 반경(half-res 픽셀)
float g_fDoFAutoFocus = 1.f; // 1=화면 중앙 깊이로 자동 초점

/* SSR 폴백용 IBL */
TextureCube g_PrefilteredCube; // 디퍼드와 동일 큐브
int g_iSpecularMip = 1;
float g_fIBLIntensity = 0.f;
float4x4 g_CamViewMatrixInverse;

//ToneMapping

float g_fExposure = 1.0f; // 커비 ToneMapping 노출 스칼라
float g_fToneMapMode = 1.0f; // 0=Reinhard(기존) / 1=ACES / 2=노출만(커비 literal)

//ui커튼
Texture2D g_CurtainTexture;

Texture3D g_ColorGradingLUT;
float g_fColorGradeEnable = 0.f;
float g_fSaturation = 0.88f;

Texture2D<uint> g_MaterialIDTexture; // R8_UINT, matID 전용

float g_fSpotlightDarken;

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

//DoF 헬퍼
float DoF_ViewZ(float2 uv)
{
    float ndcZ = g_DepthTexture.SampleLevel(LinearSampler, uv, 0).x; // z/w
    return ViewPosFromUV(uv, ndcZ).z;
}
float DoF_Focus()
{
    return (g_fDoFAutoFocus > 0.5f) ? DoF_ViewZ(float2(0.5f, 0.5f)) : g_fFocusDist;
}
float DoF_CoC(float2 uv, float focus) // 음수=near, 양수=far, [-1,1]
{
    float vz = max(DoF_ViewZ(uv), 1e-3f);
    return clamp((1.f - focus / vz) * g_fAperture, -1.f, 1.f);
}

// Tone 매핑
float3 ACESFilm(float3 x) // Narkowicz ACES 근사
{
    return saturate((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f));
}
float3 ApplySaturation(float3 c, float s)
{
    float luma = dot(c, float3(0.2126f, 0.7152f, 0.0722f)); //Rec.709
    return lerp(luma.xxx, c, s);
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
    float3 bloom = g_BloomTexture.Sample(LinearSampler, In.vTexcoord).rgb;

    float3 color = scene.rgb + bloom * g_fBloomIntensity;
    color *= g_fExposure;

    if (g_fColorGradeEnable > 0.5f)
    {
      // 선형 노출색 → LUT (톤+감마+그레이드 내장). saturate로 하드클립(커비와 동일)
        float3 uvw = saturate(color) * 0.9375f + 0.03125f;
        color = g_ColorGradingLUT.SampleLevel(ClampSampler, uvw, 0).rgb;
    }
    else
    {
      // LUT 없을 때만 자체 톤맵 + 감마
        if (g_fToneMapMode < 0.5f)
            color = color / (color + 1.f);
        else if (g_fToneMapMode < 1.5f)
            color = ACESFilm(color);
        color = pow(saturate(color), 1.f / 2.2f);
    }
    color = ApplySaturation(color, g_fSaturation);
    
    return float4(color, 1.f);
}

  //============================ SSAO (pass 3) ============================
float4 PS_SSAO(PS_IN In) : SV_TARGET0
{
    float ndcZ = g_DepthTexture.Sample(PointSampler, In.vTexcoord).x;
    if (ndcZ == 0.f)              // 배경 → 차폐 없음
        return 1.f.xxxx;

    float3 viewPos = ViewPosFromUV(In.vTexcoord, ndcZ);

    float3 worldGeoN = normalize(g_GeoNormalTexture.Sample(PointSampler, In.vTexcoord).xyz * 2.f - 1.f);
    float3 viewN = normalize(mul(worldGeoN, (float3x3) g_CamViewMatrix));

      // 원근 보정: 멀수록 반경/바이어스를 키워서 화면상 일정 + 그레이징 자기차폐 방지
    float depthScale = max(viewPos.z, 1.f) * 0.01f; // 0.01 계수는 디버그뷰 보며 튜닝
    float radius = g_fSSAORadius * depthScale;
    float bias = g_fSSAOBias * depthScale;

      // 표면에서 노멀 방향으로 살짝 띄운 원점(노멀/깊이 노이즈로 인한 자기차폐 차단)
    float3 origin = viewPos + viewN * bias;

    const int KERNEL = 16;
    float occlusion = 0.f;
      [unroll]
    for (int i = 0; i < KERNEL; ++i)
    {
        float3 dir = normalize(Hash3(In.vTexcoord, (float) i)); // 방향 정규화
        if (dot(dir, viewN) < 0.f)
            dir = -dir;
        float scale = (float) i / KERNEL;
        scale = lerp(0.1f, 1.f, scale * scale);
        float3 samplePos = origin + dir * scale * radius; // viewPos 대신 띄운 origin 기준

        float4 offset = mul(float4(samplePos, 1.f), g_CamProjMatrix);
        offset.xyz /= offset.w;
        float2 sUV = offset.xy * float2(0.5f, -0.5f) + 0.5f;
        if (sUV.x < 0.f || sUV.x > 1.f || sUV.y < 0.f || sUV.y > 1.f)
            continue;

        float sceneZ = ViewPosFromUV(sUV, g_DepthTexture.Sample(PointSampler, sUV).x).z;
        float rangeCheck = smoothstep(0.f, 1.f, radius / max(abs(viewPos.z - sceneZ), 1e-4f));
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

    // 변경: iblSpec은 이미 scene에 포함. SSR 맞은 곳만 그쪽으로 교체.
    //       (iblSpec 계산은 빼주려고 그대로 유지)
    return float4(scene.rgb + (ssrColor - iblSpec) * conf, scene.a);
}

// Downsample + CoC → half. rgb=color, a=coc*0.5+0.5 (6)
float4 PS_DOF_DOWN(PS_IN In) : SV_TARGET0
{
    float coc = DoF_CoC(In.vTexcoord, DoF_Focus());
    float3 col = g_SceneTexture.SampleLevel(LinearSampler, In.vTexcoord, 0).rgb;
    return float4(col, coc * 0.5f + 0.5f);
}

  // Scatter-as-gather blur → half. g_BloomTexture = Target_DoF_Down (7)
static const int DOF_TAPS = 16;
float4 PS_DOF_BLUR(PS_IN In) : SV_TARGET0
{
    float4 c0 = g_BloomTexture.SampleLevel(LinearSampler, In.vTexcoord, 0);
    float3 sum = 0.f;
    float wsum = 0.f;
    float cocSum = 0.f;
      [unroll]
    for (int i = 0; i < DOF_TAPS; ++i)
    {
        float t = (i + 0.5f) / DOF_TAPS;
        float ang = t * 6.2831853f * 4.0f; // 나선
        float rad = sqrt(t); // 디스크 균등 분포
        float2 off = float2(cos(ang), sin(ang)) * rad * g_fDoFMaxCoC * g_vTexelSize;
        float4 s = g_BloomTexture.SampleLevel(LinearSampler, In.vTexcoord + off, 0);
        float scoc = s.a * 2.f - 1.f;
          // '샘플'의 blur 반경이 중심까지 닿으면 기여 → near/far 모두 번짐
        float w = saturate(abs(scoc) * g_fDoFMaxCoC - rad * g_fDoFMaxCoC + 1.f);
        sum += s.rgb * w;
        wsum += w;
        cocSum += scoc * w;
    }
    float3 col = (wsum > 1e-4f) ? sum / wsum : c0.rgb;
    float bledCoC = (wsum > 1e-4f) ? cocSum / wsum : (c0.a * 2.f - 1.f);
    return float4(col, bledCoC * 0.5f + 0.5f);
}

  // Composite → full. g_SceneTexture=sharp(SSR), g_BloomTexture=blurred(half) (8)
float4 PS_DOF_COMPOSITE(PS_IN In) : SV_TARGET0
{
    float4 sharp = g_SceneTexture.SampleLevel(LinearSampler, In.vTexcoord, 0);
    if (g_fDoFEnable < 0.5f)
        return sharp;

    float4 b = g_BloomTexture.SampleLevel(LinearSampler, In.vTexcoord, 0); // 업샘플
    float coc = abs(DoF_CoC(In.vTexcoord, DoF_Focus())); // 이 픽셀 흐림량
    float bled = b.a * 2.f - 1.f; // 번져 들어온 CoC
    float nearBleed = saturate(-bled); // 앞 물체가 위로 번짐
    float tt = max(smoothstep(0.05f, 0.5f, coc), nearBleed);
    return float4(lerp(sharp.rgb, b.rgb, saturate(tt)), sharp.a);
}

float4 PS_CURTAIN_COMPOSITE(PS_IN In) : SV_TARGET0
{
    return g_CurtainTexture.Sample(LinearSampler, In.vTexcoord);
}

float4 PS_OCCLUSION_SILHOUETTE(PS_IN In) : SV_TARGET0
{
    uint matID = g_MaterialIDTexture.Load(int3(In.vPosition.xy, 0));

    if (matID == 0)
        discard;

    return float4(0.f, 0.f, 0.f, 0.55f);
}

float4 PS_SPOTLIGHT_DARKEN(PS_IN In) : SV_TARGET0
{
    return float4(0.f, 0.f, 0.f, g_fSpotlightDarken);
}

float4 PS_ESM_RESOLVE(PS_IN In) : SV_TARGET0
{
    float d = g_LightDepthTexture.Sample(LinearSampler, In.vTexcoord).r;
    return float4(exp(g_fESMConst * d), 0.f, 0.f, 1.f);
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
    pass DoF_Down // 6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOF_DOWN();
    }
    pass DoF_Blur // 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOF_BLUR();
    }
    pass DoF_Composite // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOF_COMPOSITE();
    }

    pass CurtainComposite // 9
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0, 0, 0, 0), 0xffffffff); // 알파오버
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CURTAIN_COMPOSITE();
    }

    pass OcclusionSilhouette // 10
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_StencilEqual, 1); // 스텐실==1 만
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN(); // 기존 풀스크린 쿼드 VS 그대로
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OCCLUSION_SILHOUETTE();
    }

    pass SpotlightDarken // 11
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SpotlightDarken, 1); // ref=1, stencil!=1 인 곳만 통과
        SetBlendState(BS_AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SPOTLIGHT_DARKEN();
    }
    pass ESM_Resolve // 12 (enum ESM_RESOLVE)
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ESM_RESOLVE();
    }
}