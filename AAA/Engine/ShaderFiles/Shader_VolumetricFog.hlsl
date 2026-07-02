
static const float PI = 3.14159265f;

  // FROXEL_CB (C++ Engine_Struct.h) 와 1:1 미러. XMFLOAT4X4=행우선 → row_major
cbuffer FroxelCB : register(b0)
{
    row_major float4x4 g_CamViewInv;
    row_major float4x4 g_CamProjInv;
    row_major float4x4 g_ShadowView;
    row_major float4x4 g_ShadowProj;

    float4 g_vCamPos; // xyz
    float4 g_vLightDir; // xyz : 빛이 진행하는 방향(씬을 향함)
    float4 g_vLightColor; // rgb * 강도
    float4 g_vFogScatter; // rgb=산란 알베도, a=기준 밀도
    float4 g_vFogParams; // x=near y=far z=heightFalloff w=baseHeight
    float4 g_vFogParams2; // x=g(이방성) y=ambient z=time w=shadowStrength
    float4 g_vGridParams; // x=W y=H z=D w=jitter
    float4 g_vFogParams3; // x=fogStart, y=startRange
};

Texture2D g_ShadowDepth : register(t0); // Target_LightDepth (.r)
SamplerState g_ShadowSampler : register(s0); // border=1 (clamp)

RWTexture3D<float4> g_Scatter : register(u0); // Inject 출력 : (sigma_s*inScatter, sigma_t)
Texture3D<float4> g_ScatterIn : register(t1); // Integrate 입력 (Scatter SRV)
RWTexture3D<float4> g_Integrated : register(u0); // Integrate 출력 : (누적 inScatter, 투과율)

  // 헬퍼
float HenyeyGreenstein(float cosT, float g)
{
    float g2 = g * g;
    float denom = 1.f + g2 - 2.f * g * cosT;
    return (1.f - g2) / (4.f * PI * pow(max(denom, 1e-4f), 1.5f));
}

float Hash3D(float3 p)
{
    p = frac(p * 0.3183099f + 0.1f);
    p *= 17.f;
    return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
}
float ValueNoise3D(float3 x)
{
    float3 i = floor(x);
    float3 f = frac(x);
    f = f * f * (3.f - 2.f * f);
    float n000 = Hash3D(i + float3(0, 0, 0)), n100 = Hash3D(i + float3(1, 0, 0));
    float n010 = Hash3D(i + float3(0, 1, 0)), n110 = Hash3D(i + float3(1, 1, 0));
    float n001 = Hash3D(i + float3(0, 0, 1)), n101 = Hash3D(i + float3(1, 0, 1));
    float n011 = Hash3D(i + float3(0, 1, 1)), n111 = Hash3D(i + float3(1, 1, 1));
    float nx00 = lerp(n000, n100, f.x), nx10 = lerp(n010, n110, f.x);
    float nx01 = lerp(n001, n101, f.x), nx11 = lerp(n011, n111, f.x);
    float nxy0 = lerp(nx00, nx10, f.y), nxy1 = lerp(nx01, nx11, f.y);
    return lerp(nxy0, nxy1, f.z);
}

  // froxel z슬라이스 → 뷰공간 선형깊이 (지수분포)
float SliceViewZ(float frac01)
{
    float zn = g_vFogParams.x, zf = g_vFogParams.y;
    return zn * pow(zf / zn, frac01);
}

  // froxel(uv, viewZ) → 월드좌표
float3 FroxelToWorld(float2 uv, float viewZ)
{
    float2 ndc = float2(uv.x * 2.f - 1.f, uv.y * -2.f + 1.f);
    float4 pFar = mul(float4(ndc, 1.f, 1.f), g_CamProjInv); // ndcZ=1 뷰공간 점
    float3 viewDir = pFar.xyz / pFar.w; // 뷰 +z = 전방
    float3 viewPos = viewDir * (viewZ / viewDir.z);
    return mul(float4(viewPos, 1.f), g_CamViewInv).xyz;
}

float SampleShadow(float3 worldPos)
{
    float4 lc = mul(float4(worldPos, 1.f), g_ShadowView);
    lc = mul(lc, g_ShadowProj);
    if (lc.w <= 0.f)
        return 1.f;
    float2 suv = float2(lc.x / lc.w * 0.5f + 0.5f, lc.y / lc.w * -0.5f + 0.5f);
    float pz = lc.z / lc.w;
    if (suv.x < 0.f || suv.x > 1.f || suv.y < 0.f || suv.y > 1.f || pz > 1.f)
        return 1.f;
    float esm = g_ShadowDepth.SampleLevel(g_ShadowSampler, suv, 0).r;
    return saturate(exp(-80.f * pz) * esm);
}

  //============================================================
  //  Pass 1 : Inject
  //============================================================
[numthreads(8, 8, 8)]
  void CS_Inject(uint3 id : SV_DispatchThreadID)
{
    uint W = (uint) g_vGridParams.x, H = (uint) g_vGridParams.y, D = (uint) g_vGridParams.z;
    if (id.x >= W || id.y >= H || id.z >= D)
        return;

    float2 uv = float2((id.x + 0.5f) / W, (id.y + 0.5f) / H);
    float viewZ = SliceViewZ((id.z + 0.5f + g_vGridParams.w) / D);
    float3 wp = FroxelToWorld(uv, viewZ);

      // 높이 기반 밀도 + 3D 노이즈
    float heightDensity = exp(-g_vFogParams.z * (wp.y - g_vFogParams.w));
    float noise = ValueNoise3D(wp * 0.15f + float3(g_vFogParams2.z * 0.05f, 0.f, g_vFogParams2.z * 0.03f));
    float density = g_vFogScatter.a * saturate(heightDensity) * lerp(0.6f, 1.4f, noise);
    
    // 시작 거리 게이트: fogStart 전엔 밀도 0, startRange 동안 램프업
    float startGate = saturate((viewZ - g_vFogParams3.x) / max(g_vFogParams3.y, 1e-3f));
    density *= startGate;

    float extinction = max(density, 1e-5f);
    float3 scattering = g_vFogScatter.rgb * density;

      // 디렉셔널 in-scatter (HG phase + 그림자)
    float3 rayDir = normalize(wp - g_vCamPos.xyz);
    float cosT = dot(-rayDir, normalize(g_vLightDir.xyz));
    float phase = HenyeyGreenstein(cosT, g_vFogParams2.x);

    float shadow = SampleShadow(wp);
    float shadowT = lerp(1.f, shadow, g_vFogParams2.w);

    float3 inScatter = g_vLightColor.rgb * phase * shadowT + g_vFogParams2.y.xxx;

    g_Scatter[id] = float4(scattering * inScatter, extinction);
}

  //============================================================
  //  Pass 2 : Integrate (z축 front-to-back)
  //============================================================
[numthreads(8, 8, 1)]
  void CS_Integrate(uint3 id : SV_DispatchThreadID)
{
    uint W = (uint) g_vGridParams.x, H = (uint) g_vGridParams.y, D = (uint) g_vGridParams.z;
    if (id.x >= W || id.y >= H)
        return;

    float3 accumScatter = 0.f.xxx;
    float accumTrans = 1.f;
    float prevViewZ = g_vFogParams.x; // near

      [loop]
    for (uint z = 0; z < D; ++z)
    {
        float4 s = g_ScatterIn.Load(int4(id.xy, z, 0));
        float3 scattering = s.rgb;
        float extinction = max(s.a, 1e-5f);

        float viewZ = SliceViewZ((z + 1.f) / D);
        float stepLen = max(viewZ - prevViewZ, 0.f);
        prevViewZ = viewZ;

        float sliceTrans = exp(-extinction * stepLen);
          // 에너지 보존 적분(Frostbite)
        float3 integScatter = (scattering - scattering * sliceTrans) / extinction;
        accumScatter += accumTrans * integScatter;
        accumTrans *= sliceTrans;

        g_Integrated[uint3(id.xy, z)] = float4(accumScatter, accumTrans);
    }
}