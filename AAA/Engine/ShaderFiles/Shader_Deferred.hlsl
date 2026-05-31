#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_ShadowLightViewMatrix, g_ShadowLightProjMatrix;
float4x4 g_ViewMatrixInverse, g_ProjMatrixInverse;

  /* G-buffer */
Texture2D g_Texture; // 디버그용
Texture2D g_DiffuseTexture; // albedo (rgb), alpha
Texture2D g_NormalTexture; // world normal * 0.5 + 0.5
Texture2D g_DepthTexture; // (z/w, viewZ/500, ...)
Texture2D g_MRATexture; // r=metallic, g=roughness, b=ao
Texture2D g_LightTexture; // HDR 광량 누적 (Combine 입력)
Texture2D g_LightDepthTexture; // 그림자맵

vector g_vCamPosition;

  /* Light */
vector g_vLightDir;
vector g_vLightPos;
float g_fLightRange;
vector g_vLightDiffuse;
vector g_vLightAmbient;
vector g_vLightSpecular;

  /* Combine 전용 앰비언트 (바인딩 안 해도 기본값 사용) */
vector g_vAmbientColor = float4(0.15f, 0.15f, 0.18f, 1.f);

static const float PI = 3.14159265f;

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

  //============================ PBR Helpers ============================
float3 Fresnel(float3 F0, float cosT)
{
    return F0 + (1.f - F0) * pow(1.f - cosT, 5.f);
}
float DistGGX(float3 N, float3 H, float r)
{
    float a = r * r, a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float d = NdotH * NdotH * (a2 - 1.f) + 1.f;
    return a2 / max(PI * d * d, 1e-5);
}
float GeomSchlick(float NdotX, float r)
{
    float k = (r + 1.f);
    k = k * k / 8.f;
    return NdotX / (NdotX * (1.f - k) + k);
}
float GeomSmith(float3 N, float3 V, float3 L, float r)
{
    return GeomSchlick(saturate(dot(N, V)), r) * GeomSchlick(saturate(dot(N, L)), r);
}

float3 RecoverWorldPos(float2 uv, float depthZ, float viewZ)
{
    float4 p;
    p.x = uv.x * 2.f - 1.f;
    p.y = uv.y * -2.f + 1.f;
    p.z = depthZ;
    p.w = 1.f;
    p *= viewZ;
    p = mul(p, g_ProjMatrixInverse);
    p = mul(p, g_ViewMatrixInverse);
    return p.xyz;
}

float3 CookTorrance(float3 N, float3 V, float3 L, float3 albedo,
                      float metallic, float roughness, float3 radiance)
{
    roughness = clamp(roughness, 0.04f, 1.f);
    float3 H = normalize(V + L);
    float3 F0 = lerp(0.04f, albedo, metallic);
    float3 F = Fresnel(F0, saturate(dot(H, V)));
    float D = DistGGX(N, H, roughness);
    float G = GeomSmith(N, V, L, roughness);

    float3 spec = (D * G * F) /
          max(4.f * saturate(dot(N, V)) * saturate(dot(N, L)), 1e-4);

    float3 kD = (1.f - F) * (1.f - metallic);
    float3 diff = kD * albedo / PI;

    return (diff + spec) * radiance * saturate(dot(N, L));
}

  //============================ Debug (pass 0) ============================
float4 PS_MAIN_DEBUG(PS_IN In) : SV_TARGET0
{
    return g_Texture.Sample(LinearSampler, In.vTexcoord);
}

  //============================ Directional (pass 1) ============================
float4 PS_MAIN_DIRECTIONAL(PS_IN In) : SV_TARGET0
{
    float4 nd = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    float4 dd = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float3 albedo = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float3 mra = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

    float3 N = normalize(nd.xyz * 2.f - 1.f);
    float3 wp = RecoverWorldPos(In.vTexcoord, dd.x, dd.y * 500.f);
    float3 V = normalize(g_vCamPosition.xyz - wp);
    float3 L = normalize(-g_vLightDir.xyz);

    float3 Lo = CookTorrance(N, V, L, albedo, mra.r, mra.g, g_vLightDiffuse.rgb);
    return float4(Lo, 1.f);
}

  //============================ Point (pass 2) ============================
float4 PS_MAIN_POINT(PS_IN In) : SV_TARGET0
{
    float4 nd = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    float4 dd = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float3 albedo = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float3 mra = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

    float3 N = normalize(nd.xyz * 2.f - 1.f);
    float3 wp = RecoverWorldPos(In.vTexcoord, dd.x, dd.y * 500.f);
    float3 V = normalize(g_vCamPosition.xyz - wp);

    float3 Lvec = g_vLightPos.xyz - wp;
    float dist = length(Lvec);
    float att = saturate((g_fLightRange - dist) / g_fLightRange);
    att *= att;
    float3 L = Lvec / max(dist, 1e-4);

    float3 Lo = CookTorrance(N, V, L, albedo, mra.r, mra.g, g_vLightDiffuse.rgb) * att;
    return float4(Lo, 1.f);
}

  //============================ Combined (pass 3) ============================
float4 PS_MAIN_COMBINED(PS_IN In) : SV_TARGET0
{
    float4 albedoA = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == albedoA.a)
        discard;

    float3 light = g_LightTexture.Sample(LinearSampler, In.vTexcoord).rgb;
    float3 mra = g_MRATexture.Sample(LinearSampler, In.vTexcoord).rgb;

      /* 앰비언트 (추후 IBL로 교체) */
    float3 ambient = albedoA.rgb * g_vAmbientColor.rgb * mra.b; // mra.b = AO
    float3 color = light + ambient;

      /* 그림자 */
    float4 dd = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float3 wp = RecoverWorldPos(In.vTexcoord, dd.x, dd.y * 500.f);
    float4 lc = mul(float4(wp, 1.f), g_ShadowLightViewMatrix);
    lc = mul(lc, g_ShadowLightProjMatrix);
    float2 suv = float2(lc.x / lc.w * 0.5f + 0.5f, lc.y / lc.w * -0.5f + 0.5f);
    float pz = lc.z / lc.w;
    float sd = g_LightDepthTexture.Sample(BorderSampler, suv).r;
    if (pz <= 1.f && pz - 0.002f > sd)
        color *= 0.5f;

      /* 톤매핑 + 감마 */
    color = color / (color + 1.f); // Reinhard
    color = pow(color, 1.f / 2.2f); // 감마
    return float4(color, 1.f);
}

  //============================ Technique ============================
technique11 DefaultTechnique
{
    pass Debug // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }
    pass Directional // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }
    pass Point // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }
    pass Combined // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }
}