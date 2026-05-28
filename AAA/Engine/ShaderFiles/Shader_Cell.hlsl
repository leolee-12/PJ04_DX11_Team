#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4 g_vColor = float4(0.f, 1.f, 0.f, 0.f);

struct VS_IN
{
    float3 vPosition : POSITION;    
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;

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
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};
    

/* 픽셀셰이더 : 픽셀의 최종적인 색을 결정해준다. */
PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_vColor;    
    
    return Out;
}


technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}