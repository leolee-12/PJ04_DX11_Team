#define MATID_NORM(id)        ((id) / 255.f)
#define MATID_EQ(sampledA, id) (abs((sampledA) - MATID_NORM(id)) <= (0.5f / 255.f))

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

SamplerState MirrorSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = MIRROR;
    AddressV = MIRROR;
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

DepthStencilState DSS_MarkOccluded
{
    DepthEnable = true;
    DepthWriteMask = All;
    DepthFunc = less_equal;

    StencilEnable = true;
    StencilReadMask = 0xFF;
    StencilWriteMask = 0xFF;

    FrontFaceStencilFunc = Always;
    FrontFaceStencilPass = Replace; // 보이는 커비도 기록 (Keep 에서 변경)
    FrontFaceStencilDepthFail = Replace; // 가려진 커비도 기록 (기존)
    FrontFaceStencilFail = Keep;

    BackFaceStencilFunc = Always;
    BackFaceStencilPass = Replace; // 변경
    BackFaceStencilDepthFail = Replace;
    BackFaceStencilFail = Keep;
};

// 실루엣 합성용: 스텐실 == ref(1) 인 픽셀만 통과, 깊이 무시
DepthStencilState DSS_StencilEqual
{
    DepthEnable = false;
    DepthWriteMask = Zero;

    StencilEnable = true;
    StencilReadMask = 0xFF;
    StencilWriteMask = 0x00; // 읽기만

    FrontFaceStencilFunc = Equal;
    FrontFaceStencilPass = Keep;
    FrontFaceStencilFail = Keep;
    FrontFaceStencilDepthFail = Keep;

    BackFaceStencilFunc = Equal;
    BackFaceStencilPass = Keep;
    BackFaceStencilFail = Keep;
    BackFaceStencilDepthFail = Keep;
};

DepthStencilState DSS_SpotlightDarken
{
    DepthEnable = false;
    DepthWriteMask = Zero;

    StencilEnable = true;
    StencilReadMask = 0xFF;
    StencilWriteMask = 0x00;

    FrontFaceStencilFunc = Not_Equal; // stencil != ref(1) => 배경/월드
    FrontFaceStencilPass = Keep;
    FrontFaceStencilFail = Keep;
    FrontFaceStencilDepthFail = Keep;

    BackFaceStencilFunc = Not_Equal;
    BackFaceStencilPass = Keep;
    BackFaceStencilFail = Keep;
    BackFaceStencilDepthFail = Keep;
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

    SrcBlend = Src_Alpha;
    DestBlend = One;
    BlendOp = Add;

    SrcBlendAlpha = Src_Alpha; // 스타 커버리지를 알파에 더함
    DestBlendAlpha = One; // 기존 씬 알파(지형=1)를 절대 깎지 않음
    BlendOpAlpha = Add; // dstA = srcA + dstA
};

BlendState BS_Max
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = One;
    DestBlend = One;
    BlendOp = Max;

    SrcBlendAlpha = One;
    DestBlendAlpha = One;
    BlendOpAlpha = Max;
};

// 커튼 RT 용
BlendState BS_CurtainOver
{
    BlendEnable[0] = true;
    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;
    SrcBlendAlpha = One; // dstA = srcA + (1-srcA)*dstA
    DestBlendAlpha = Inv_Src_Alpha;
    BlendOpAlpha = Add;
};

// 지우개: RGB는 그대로, 대상 알파만 별 모양으로 깎음
BlendState BS_AlphaErase
{
    BlendEnable[0] = true;
    SrcBlend = Zero; // RGB 유지
    DestBlend = One;
    BlendOp = Add;
    SrcBlendAlpha = Zero; // dstA = (1-srcA)*dstA
    DestBlendAlpha = Inv_Src_Alpha;
    BlendOpAlpha = Add;
};

BlendState BS_Decal   // 데칼: Diffuse/Normal/MRA 3타깃 알파블렌드, matID는 MRT에 없어 안 건드림
{
    BlendEnable[0] = true;
    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = Inv_Src_Alpha;
    BlendOp[0] = Add;
    SrcBlendAlpha[0] = Zero;
    DestBlendAlpha[0] = One;
    BlendOpAlpha[0] = Add;

    BlendEnable[1] = true;
    SrcBlend[1] = Src_Alpha;
    DestBlend[1] = Inv_Src_Alpha;
    BlendOp[1] = Add;
    SrcBlendAlpha[1] = Zero;
    DestBlendAlpha[1] = One;
    BlendOpAlpha[1] = Add;

    BlendEnable[2] = true;
    SrcBlend[2] = Src_Alpha;
    DestBlend[2] = Inv_Src_Alpha;
    BlendOp[2] = Add;
    SrcBlendAlpha[2] = Zero;
    DestBlendAlpha[2] = One;
    BlendOpAlpha[2] = Add;
};


// 헬퍼
float3 RecoverWorldPos(float2 uv, float depthZ, float4x4 projInv, float4x4 viewInv)
{
    float4 p;
    p.x = uv.x * 2.f - 1.f;
    p.y = uv.y * -2.f + 1.f;
    p.z = depthZ;
    p.w = 1.f;
    p = mul(p, projInv);
    p /= p.w;
    p = mul(float4(p.xyz, 1.f), viewInv);
    return p.xyz;
}
