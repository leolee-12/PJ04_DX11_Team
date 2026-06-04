#include "Engine_Shader_Defines.hlsli"
float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

struct VS_IN
{
    float3 vPosition : POSITION;
    float4 vColor : COLOR;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vColor : COLOR;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    float4 vPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vPos = mul(vPos, g_ViewMatrix);
    vPos = mul(vPos, g_ProjMatrix);
    Out.vPosition = vPos;
    Out.vColor = In.vColor;
    return Out;
}

float4 PS_MAIN(VS_OUT In) : SV_TARGET0
{
    return In.vColor;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}