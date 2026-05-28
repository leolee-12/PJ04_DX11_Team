#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_ShadowLightViewMatrix, g_ShadowLightProjMatrix;
float4x4 g_ViewMatrixInverse, g_ProjMatrixInverse;

Texture2D g_Texture;
Texture2D g_NormalTexture;
Texture2D g_DiffuseTexture;
Texture2D g_ShadeTexture;
Texture2D g_DepthTexture;
Texture2D g_SpecularTexture;
Texture2D g_LightDepthTexture;
Texture2D g_OutlineMaskTexture;

vector g_vCamPosition;

vector g_vLightDir;
vector g_vLightPos;
float g_fLightRange;

vector g_vLightDiffuse;
vector g_vLightAmbient;
vector g_vLightSpecular;

vector g_vMtrlAmbient = 1.f;
vector g_vMtrlSpecular = 1.f;

float2 g_vTexel;

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
    

/* 정점셰이더 : 정점 데이터의 변환 과정을 수행한다. */

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    /* 월드변환, 뷰 벼환, 투영변환 */ 
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

struct PS_OUT_BACKBUFFER
{
    float4 vBackBuffer : SV_TARGET0;
};
    


PS_OUT_BACKBUFFER PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    Out.vBackBuffer = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    return Out;
}

struct PS_OUT_LIGHT
{
    float4 vShade : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
};
    

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;
    
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    Out.vShade = g_vLightDiffuse * (saturate(dot(normalize(g_vLightDir) * -1.f, normalize(vNormal))) + (g_vLightAmbient * g_vMtrlAmbient));
    
    vector vReflect = reflect(normalize(g_vLightDir), normalize(vNormal));
    
    vector vWorldPos;
    
    /* 투영공간상의 위치 */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 뷰스페이스 상의 위치 */
    vWorldPos *= fViewZ;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 월드스페이스 상의 위치 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
        
    vector vLook = vWorldPos - g_vCamPosition;
    Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) *
        pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))), 50.f);
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;
    
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    vector vWorldPos;
    
    /* 투영공간상의 위치 */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 뷰스페이스 상의 위치 */
    vWorldPos *= fViewZ;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 월드스페이스 상의 위치 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
        
    vector vLightDir = vWorldPos - g_vLightPos;
    
    float fAtt = saturate((g_fLightRange - length(vLightDir)) / g_fLightRange);
    
    Out.vShade = (g_vLightDiffuse * (saturate(dot(normalize(vLightDir) * -1.f, normalize(vNormal))) + (g_vLightAmbient * g_vMtrlAmbient))) * fAtt;
    
    vector vReflect = reflect(normalize(vLightDir), normalize(vNormal));
    
    
    vector vLook = vWorldPos - g_vCamPosition;
    Out.vSpecular = ((g_vLightSpecular * g_vMtrlSpecular) *
        pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))), 50.f)) * fAtt;
    
    return Out;
}


PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == vDiffuse.a)
        discard;
    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);
    
    Out.vBackBuffer = vDiffuse * vShade + vSpecular;
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;

    vector vWorldPos;
    
    /* 투영공간상의 위치 */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 뷰스페이스 상의 위치 */
    vWorldPos *= fViewZ;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    /* 월드스페이스 상의 위치 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    float4 vLightClip = mul(float4(vWorldPos.xyz, 1.f), g_ShadowLightViewMatrix);
    vLightClip = mul(vLightClip, g_ShadowLightProjMatrix);

    float2 vTexcoord;
    vTexcoord.x = (vLightClip.x / vLightClip.w) * 0.5f + 0.5f;
    vTexcoord.y = (vLightClip.y / vLightClip.w) * -0.5f + 0.5f;

    float fLightProjZ = vLightClip.z / vLightClip.w; 
    float fSampledDepth = g_LightDepthTexture.Sample(BorderSampler, vTexcoord).r;

    float fShadowBias = 0.002f;
    if (fLightProjZ <= 1.f && fLightProjZ - fShadowBias > fSampledDepth)
        Out.vBackBuffer *= 0.5f;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_OUTLINE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;

    float2 self = g_OutlineMaskTexture.Sample(LinearSampler, In.vTexcoord).rg;
    if (self.r > 0.01f || self.g > 0.01f)
        discard;

    static const float2 dirs[8] =
    {
        float2(1, 0), float2(-1, 0), float2(0, 1), float2(0, -1),
          float2(1, 1), float2(-1, 1), float2(1, -1), float2(-1, -1)
    };

    float thinR = 0.f, thickR = 0.f;
    float thinG = 0.f, thickG = 0.f;

      [unroll]
    for (int i = 0; i < 8; ++i)
    {
        float2 d = dirs[i];

        float2 n1 = g_OutlineMaskTexture.Sample(LinearSampler, In.vTexcoord + d * g_vTexel).rg;
        if (n1.r > 0.4f && n1.r < 0.6f)
            thinR = 1.f;
        if (n1.g > 0.4f && n1.g < 0.6f)
            thinG = 1.f;

          [unroll]
        for (int r = 1; r <= 2; ++r)
        {
            float2 n = g_OutlineMaskTexture.Sample(LinearSampler, In.vTexcoord + d * g_vTexel * r).rg;
            if (n.r > 0.9f)
                thickR = 1.f;
            if (n.g > 0.9f)
                thickG = 1.f;
        }
    }

    if (thinR + thickR + thinG + thickG < 0.01f)
        discard;
    
    float3 color = (thinG + thickG > 0.5f) ? float3(1.f, 0.f, 0.f) : float3(1.f, 1.f, 1.f);
    Out.vBackBuffer = float4(color, 1.f);
    return Out;
}

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
    pass Outline // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_OUTLINE();
    }

}