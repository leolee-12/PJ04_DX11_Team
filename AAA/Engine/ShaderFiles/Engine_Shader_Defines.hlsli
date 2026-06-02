
SamplerState LinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};
SamplerState ClampSampler
{
    filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

SamplerState BorderSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = float4(1.f, 1.f, 1.f, 1.f);
};

SamplerState PointSampler
{
    filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
};

SamplerState UISampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};




RasterizerState RS_Wireframe
{
    FillMode = WireFrame;
};

RasterizerState RS_Default
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
};

RasterizerState RS_Cull_CW
{
    FillMode = Solid;
    CullMode = Front;
    FrontCounterClockwise = false;
};

RasterizerState RS_Cull_None
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
};

RasterizerState RS_Decal
{
    FillMode = Solid;
    CullMode = Back;
    DepthBias = -1000; // 카메라 쪽으로 살짝 당겨 z-fighting 방지
    SlopeScaledDepthBias = -1.f;
};

DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = All;
    DepthFunc = less_equal;
};

DepthStencilState DSS_Z_Disable
{
    DepthEnable = false;
    DepthWriteMask = Zero;  
};

DepthStencilState DSS_NoWrite
{
    DepthEnable = true;
    DepthWriteMask = Zero;
    DepthFunc = less_equal;
};


BlendState BS_Default
{
    BlendEnable[0] = false;    
    BlendEnable[1] = false;
    BlendEnable[2] = false;
    BlendEnable[3] = false;
};

BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;    
    BlendEnable[1] = true;

    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha; 
    BlendOp = Add;   

    SrcBlendAlpha = Zero; 
    DestBlendAlpha = One; 
    BlendOpAlpha = Add; 
};

BlendState BS_Additive
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = One;
    DestBlend = One;
    BlendOp = Add;
};
