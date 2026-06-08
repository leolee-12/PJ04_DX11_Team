#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_Texture;
Texture2DArray g_TextureArray;              // SpriteAnim용

float4 g_vColor = { 1.f, 1.f, 1.f, 1.f };
float g_fAlpha = { 1.f };
bool g_bAlphaTest = false; 
int g_iFrame = { 0 };                       // SpriteAnim 슬라이스 선택
float g_fTestAlpha = { 0.f };

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
    float4 vPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vPos = mul(vPos, g_ViewMatrix);
    vPos = mul(vPos, g_ProjMatrix);
    
    Out.vPosition = vPos;
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    Out.vColor = g_Texture.Sample(UISampler, In.vTexcoord);
    Out.vColor *= g_vColor;
    Out.vColor.a *= g_fAlpha;
    
    if (Out.vColor.a <= 0.f)
        discard;
    
    if (g_bAlphaTest && Out.vColor.a <= g_fTestAlpha)
        discard;
    
    return Out;
}

PS_OUT PS_SPRITEANIM(PS_IN In)
{
    PS_OUT Out;
    Out.vColor = g_TextureArray.Sample(UISampler, float3(In.vTexcoord, (float) g_iFrame));
    Out.vColor *= g_vColor;
    Out.vColor.a *= g_fAlpha;
    
    if (Out.vColor.a <= 0.f)
        discard;
    if (g_bAlphaTest && Out.vColor.a <= g_fTestAlpha)
        discard;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass UI // pass 0 
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass SpriteAnim // pass 1 
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_SPRITEANIM()));
    }
}