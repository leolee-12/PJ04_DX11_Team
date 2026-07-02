#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
bool g_bBillboard = { false };

Texture2D g_Texture;
bool g_bUseTexture = { false };
float2 g_vTextureTiling = { 1.f, 1.f };
float2 g_vTextureOffset = { 0.f, 0.f };

bool g_bSpriteAniTexture = { false };
float2 g_vSpriteAniTexUV = { 0.f, 0.f };
float2 g_vSpriteAniTexSize = { 1.f, 1.f };

Texture2D g_Mask;
bool g_bUseMask = { false };
float2 g_vMaskTiling = { 1.f, 1.f };
float2 g_vMaskOffset = { 0.f, 0.f };

bool g_bSpriteAniMask = { false };
float2 g_vSpriteAniMaskUV = { 0.f, 0.f };
float2 g_vSpriteAniMaskSize = { 1.f, 1.f };

bool g_bFlipX = { false };
bool g_bFlipY = { false };
float3 g_vColor = { 1.f, 1.f, 1.f };
float g_fAlpha = { 1 };

bool g_bAlphaTest = { false };
float g_fTestAlpha = { 0.f };

float g_fUVCutRight = { 0.f };
float g_fUVCutLeft = { 0.f };

float g_fUVCutTop = { 0.f };
float g_fUVCutBottom = { 0.f };



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

    float4 vPosition;

    if (g_bBillboard == true)
    {
        float3 vScale;
        vScale.x = length(mul(float4(1.f, 0.f, 0.f, 0.f), g_WorldMatrix).xyz);
        vScale.y = length(mul(float4(0.f, 1.f, 0.f, 0.f), g_WorldMatrix).xyz);
        vScale.z = length(mul(float4(0.f, 0.f, 1.f, 0.f), g_WorldMatrix).xyz);

        vPosition = mul(float4(0.f, 0.f, 0.f, 1.f), g_WorldMatrix);
        vPosition = mul(vPosition, g_ViewMatrix);
        vPosition.xyz += In.vPosition * vScale;
        vPosition = mul(vPosition, g_ProjMatrix);
    }
    else
    {
        vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
        vPosition = mul(vPosition, g_ViewMatrix);
        vPosition = mul(vPosition, g_ProjMatrix);
    }
    
    Out.vPosition = vPosition;
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
       
    if (In.vTexcoord.x > (1.f - g_fUVCutRight) || In.vTexcoord.x < g_fUVCutLeft)
        discard;
    
    if (In.vTexcoord.y > (1.f - g_fUVCutBottom) || In.vTexcoord.y < g_fUVCutTop)
        discard;
    
    if (g_bFlipX == 1)
        In.vTexcoord.x = -In.vTexcoord.x + 1.f;
    
    if (g_bFlipY == 1)
        In.vTexcoord.y = -In.vTexcoord.y + 1.f;
    
    Out.vColor = float4(1.f, 1.f, 1.f, 1.f);
    
    if (g_bUseTexture == true)
    {
        float2 vUV = float2(0.f, 0.f);
        
        if (g_bSpriteAniTexture == true)
            vUV = In.vTexcoord * g_vSpriteAniTexSize + g_vSpriteAniTexUV;
        else
            vUV = g_vTextureOffset + In.vTexcoord * g_vTextureTiling;
        
        Out.vColor *= g_Texture.Sample(LinearSampler, vUV);
    }
    
    if (g_bUseMask == true)
    {
        float2 vUV = float2(0.f, 0.f);
        
        if (g_bSpriteAniMask == true)
            vUV = In.vTexcoord * g_vSpriteAniMaskSize + g_vSpriteAniMaskUV;
        else
            vUV = g_vMaskOffset + In.vTexcoord * g_vMaskTiling;
        
        Out.vColor *= g_Mask.Sample(LinearSampler, vUV);
    }
    
    Out.vColor.xyz *= g_vColor;
    Out.vColor.a *= g_fAlpha;
    
    if (g_bAlphaTest == true && Out.vColor.a <= g_fTestAlpha)
        discard;
    
    return Out;
}





PS_OUT PS_MAIN_MIRROR(PS_IN In)
{
    PS_OUT Out;
       
    if (In.vTexcoord.x > (1.f - g_fUVCutRight) || In.vTexcoord.x < g_fUVCutLeft)
        discard;
    
    if (In.vTexcoord.y > (1.f - g_fUVCutBottom) || In.vTexcoord.y < g_fUVCutTop)
        discard;
    
    if (g_bFlipX == 1)
        In.vTexcoord.x = -In.vTexcoord.x + 1.f;
    
    if (g_bFlipY == 1)
        In.vTexcoord.y = -In.vTexcoord.y + 1.f;
    
    Out.vColor = float4(1.f, 1.f, 1.f, 1.f);
    
    if (g_bUseTexture == true)
    {
        float2 vUV = float2(0.f, 0.f);
        
        if (g_bSpriteAniTexture == true)
            vUV = In.vTexcoord * g_vSpriteAniTexSize + g_vSpriteAniTexUV;
        else
            vUV = g_vTextureOffset + In.vTexcoord * g_vTextureTiling;
        
        Out.vColor *= g_Texture.Sample(MirrorSampler, vUV);
    }
    
    if (g_bUseMask == true)
    {
        float2 vUV = float2(0.f, 0.f);
        
        if (g_bSpriteAniMask == true)
            vUV = In.vTexcoord * g_vSpriteAniMaskSize + g_vSpriteAniMaskUV;
        else
            vUV = g_vMaskOffset + In.vTexcoord * g_vMaskTiling;
        
        Out.vColor *= g_Mask.Sample(MirrorSampler, vUV);
    }
    
    Out.vColor.xyz *= g_vColor;
    Out.vColor.a *= g_fAlpha;
    
    if (g_bAlphaTest == true && Out.vColor.a <= g_fTestAlpha)
        discard;
    
    return Out;
}





technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass AlphaBlend
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass Additive
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass DefaultPass_Mirror
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass AlphaBlend_Mirror
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass Additive_Mirror
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoWrite, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass DefaultPass_DepthIgnore
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass AlphaBlend_DepthIgnore
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass Additive_DepthIgnore
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
    }

    pass DefaultPass_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass AlphaBlend_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }

    pass Additive_Mirror_DepthIgnore
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Z_Disable, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_MAIN_MIRROR()));
    }
}
