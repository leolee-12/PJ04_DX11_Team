#include "Renderer.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "UIObject.h"
#include "ComputeShader.h"

CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
    , m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CRenderer::Initialize()
{
    m_iRTWidth = static_cast<_uint>(m_pGameInstance_Proxy->Get_WindowWidth());
    m_iRTHeight = static_cast<_uint>(m_pGameInstance_Proxy->Get_WindowHeight());

    /* 렌더타겟들을 만든다. */
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Diffuse"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Normal"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 1.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Depth"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_MRA"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 1.f, 1.f, 0.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Emissive"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Light"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Scene"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Scene_SSR"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_BloomA"), m_iRTWidth >> 1, m_iRTHeight >> 1, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_BloomB"), m_iRTWidth >> 1, m_iRTHeight >> 1, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_SSAO"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R8_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_SSAO_Blur"), m_iRTWidth, m_iRTHeight, DXGI_FORMAT_R8_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_LightDepth"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.f, 1.f, 1.f, 1.f))))
        return E_FAIL;
    if (FAILED(Ready_DepthStencil_Buffer()))
        return E_FAIL;

    /* 만든 렌더타겟들을 장치에 동시에 바인딩되는 기준으로 모은다. */
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Diffuse"))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Normal"))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Depth"))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_MRA"))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Emissive"))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Light"))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_ShadowObjects"), TEXT("Target_LightDepth"))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_Scene"), TEXT("Target_Scene"))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_Scene_SSR"), TEXT("Target_Scene_SSR"))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_BloomA"), TEXT("Target_BloomA"))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_BloomB"), TEXT("Target_BloomB"))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_SSAO"), TEXT("Target_SSAO"))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_SSAO_Blur"), TEXT("Target_SSAO_Blur"))))
        return E_FAIL;


    m_pShaderDeferred = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
    if (nullptr == m_pShaderDeferred)
        return E_FAIL;

    m_pShaderPost = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_PostProsess.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
    if (nullptr == m_pShaderPost)
        return E_FAIL;

    m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
    if (nullptr == m_pVIBuffer)
        return E_FAIL;

    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(static_cast<_float>(m_iRTWidth), static_cast<_float>(m_iRTHeight), 1.f) * XMMatrixTranslation(0.f, 0.f, 0.1f));

#ifdef _DEBUG
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_MRA"), 150.f, 100.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Diffuse"), 150.f, 300.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Depth"), 150.f, 500.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Normal"), 150.f, 700.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Emissive"), 450.f, 100.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_LightDepth"), 450.f, 300.f, 300.f, 200.f)))
        return E_FAIL;

    //if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Light"), 150.f, 100.f, 300.f, 200.f)))
    //    return E_FAIL;
    //
    //if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Scene"), 150.f, 300.f, 300.f, 200.f)))
    //    return E_FAIL;
    //
    //if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_BloomA"), 450.f, 100.f, 300.f, 200.f)))
    //    return E_FAIL;
    //if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_BloomB"), 450.f, 300.f, 300.f, 200.f)))
    //    return E_FAIL;
    //
    //if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_SSAO"), 750.f, 100.f, 300.f, 200.f)))
    //    return E_FAIL;
    //if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_SSAO_Blur"), 750.f, 300.f, 300.f, 200.f)))
    //    return E_FAIL;
#endif

    if (FAILED(Ready_Froxel_Volumes()))
        return E_FAIL;

    return S_OK;
}

void CRenderer::Add_RenderGroup(RENDERID eGroupID, CGameObject* pGameObject)
{
    m_RenderObjects[ETOUI(eGroupID)].push_back(pGameObject);

    Safe_AddRef(pGameObject);
}

void CRenderer::Add_RenderGroup_UI(RENDERUIID eGroupID, CUIObject* pUIObject)
{
    m_RenderUIs[ETOUI(eGroupID)].push_back(pUIObject);
    Safe_AddRef(pUIObject);
}

HRESULT CRenderer::Draw()
{
    if (nullptr != m_pOutRTV)
    {
        m_pContext->OMSetRenderTargets(1, &m_pOutRTV, m_pOutDSV);
        Change_ViewportDesc(m_iOutWidth, m_iOutHeight);
    }
    else
        m_pGameInstance_Proxy->Bind_BackBuffer();

    if (FAILED(Render_Priority()))
        return E_FAIL;
    if (FAILED(Render_Shadow()))
        return E_FAIL;
    if (FAILED(Render_VolumetricFog()))
        return E_FAIL;
    if (FAILED(Render_NonBlend()))
        return E_FAIL;
    if (FAILED(Render_SSAO()))
        return E_FAIL;
    if (FAILED(Render_Lights()))
        return E_FAIL;
    if (FAILED(Render_Combined()))
        return E_FAIL;
    if (FAILED(Render_SSR()))
        return E_FAIL;
    if (FAILED(Render_Bloom()))
        return E_FAIL;

    
    if (FAILED(Render_NonLight()))
        return E_FAIL;
    if (FAILED(Render_Blend()))
        return E_FAIL;

    if (FAILED(Render_UI_BACK()))
        return E_FAIL;
    if (FAILED(Render_UI_MIDDLE()))
        return E_FAIL;
    if (FAILED(Render_UI_FRONT()))
        return E_FAIL;

#ifdef _DEBUG
    if (FAILED(Render_Debug()))
        return E_FAIL;
#endif

    return S_OK;
}

#ifdef _DEBUG
void CRenderer::Add_DebugComponent(CComponent* pComponent)
{
    if (!m_bDebugRender) return;

    m_DebugComponents.push_back(pComponent);
    Safe_AddRef(pComponent);
}
#endif

void CRenderer::Bind_RenderTarget(ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, _uint iWidth, _uint iHeight)
{
    m_pOutRTV = pRTV;
    m_pOutDSV = pDSV;
    m_iOutWidth = iWidth;
    m_iOutHeight = iHeight;
}

HRESULT CRenderer::Render_Priority()
{
    for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERID::PRIORITY)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ETOUI(RENDERID::PRIORITY)].clear();

    return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_ShadowObjects"), m_pMaxDSV)))
        return E_FAIL;

    Change_ViewportDesc(g_iMaxWidth, g_iMaxHeight);


    for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERID::SHADOW)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render_Shadow();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ETOUI(RENDERID::SHADOW)].clear();

    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    Change_ViewportDesc(Render_Width(), Render_Height());

    return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_GameObjects"))))
        return E_FAIL;

    for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERID::NONBLEND)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ETOUI(RENDERID::NONBLEND)].clear();

    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Render_SSAO()
{
    /* 1) AO 계산 → Target_SSAO */
    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_SSAO"), nullptr, false)))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_ShaderGlobals(m_pShaderPost, { "g_fSSAORadius", "g_fSSAOBias", "g_fSSAOPower" })))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderPost, "g_DepthTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Normal"), m_pShaderPost, "g_NormalTexture")))
        return E_FAIL;

    // 풀스크린 쿼드용 ORTHO
    if (FAILED(m_pShaderPost->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    // SSAO 수학용 카메라(원근) 행렬
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_CamViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_CamProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC))))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Begin(ETOUI(POSTPROSESS::SSAO))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    /* 2) 블러 → Target_SSAO_Blur */
    _float2 vTexel = { 1.f / m_iRTWidth, 1.f / m_iRTHeight};

    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_SSAO_Blur"), nullptr, false)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_SSAO"), m_pShaderPost, "g_SSAOTexture")))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_RawValue("g_vTexelSize", &vTexel, sizeof(_float2))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Begin(ETOUI(POSTPROSESS::SSAO_BLUR))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Render_Lights()
{
    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_LightAcc"))))
        return E_FAIL;

    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShaderDeferred, "g_DiffuseTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Normal"), m_pShaderDeferred, "g_NormalTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderDeferred, "g_DepthTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MRA"), m_pShaderDeferred, "g_MRATexture")))
        return E_FAIL;

    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ViewMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_RawValue("g_vCamPosition", m_pGameInstance_Proxy->Get_CamPosition(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Render_Light(m_pShaderDeferred, m_pVIBuffer)))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Render_Combined()
{
    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_Scene"), nullptr, false)))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Light"), m_pShaderDeferred, "g_LightTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShaderDeferred, "g_DiffuseTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MRA"), m_pShaderDeferred, "g_MRATexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderDeferred, "g_DepthTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_LightDepth"), m_pShaderDeferred, "g_LightDepthTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_SSAO_Blur"), m_pShaderDeferred, "g_SSAOTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Emissive"), m_pShaderDeferred, "g_EmissiveTexture")))
        return E_FAIL;

    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ShadowLightViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ShadowLightProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    const auto& env = m_pGameInstance_Proxy->Get_CurrentEnvironment();
    if (FAILED(m_pShaderDeferred->Bind_SRV("g_IrradianceCube", env.pDiffuseSRV)))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_RawValue("g_fIBLIntensity", &env.fIntensity, sizeof(_float))))
        return E_FAIL;

    //볼류메트릭포그
    if (FAILED(m_pGameInstance_Proxy->Bind_ShaderGlobals(m_pShaderDeferred, "g_fFogEnable")))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_SRV("g_FogVolume", m_pIntegSRV)))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
        return E_FAIL;
    const _float4* pFogFar = m_pGameInstance_Proxy->Get_ShaderGlobal("g_fFogFar");
    _float4 vFogDepth = _float4(0.5f, pFogFar ? pFogFar->x : 80.f, 0.f, 0.f);
    if (FAILED(m_pShaderDeferred->Bind_RawValue("g_vFogDepthParams", &vFogDepth, sizeof(_float4))))
        return E_FAIL;


    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pShaderDeferred->Begin(ETOUI(DEFERRED::COMBINED))))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Render_SSR()
{
    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_Scene_SSR"), nullptr, false)))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_ShaderGlobals(m_pShaderPost, { "g_fSSRIntensity", "g_fSSRMaxDistance", "g_fSSRThickness" })))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Scene"), m_pShaderPost, "g_SceneTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderPost, "g_DepthTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Normal"), m_pShaderPost, "g_NormalTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MRA"), m_pShaderPost, "g_MRATexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShaderPost, "g_DiffuseTexture")))
        return E_FAIL;

    if (FAILED(m_pShaderPost->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_CamViewMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_CamViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_CamProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC))))
        return E_FAIL;

    const auto& env = m_pGameInstance_Proxy->Get_CurrentEnvironment();
    if (FAILED(m_pShaderPost->Bind_SRV("g_PrefilteredCube", env.pSpecularSRV)))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_RawValue("g_iSpecularMip", &env.iSpecularMip, sizeof(_uint))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_RawValue("g_fIBLIntensity", &env.fIntensity, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_SSAO_Blur"), m_pShaderPost, "g_SSAOTexture")))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Begin(ETOUI(POSTPROSESS::SSR))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Render_VolumetricFog()
{
    const _float4* pEnable = m_pGameInstance_Proxy->Get_ShaderGlobal("g_fFogEnable");
    if (nullptr == pEnable || pEnable->x < 0.5f)
        return S_OK;

    /* 1) 디렉셔널 라이트 검색 */
    const LIGHT_DESC* pDirLight = nullptr;
    for (_uint i = 0; ; ++i)
    {
        const LIGHT_DESC* pDesc = m_pGameInstance_Proxy->Get_LightDesc(i);
        if (nullptr == pDesc)
            break;
        if (LIGHT::DIRECTIONAL == pDesc->eType)
        {
            pDirLight = pDesc;
            break;
        }
    }

    m_fFogTime += 0.016f; // 노이즈 스크롤용 (정밀 타이밍 불필요)

    /* 2) 상수버퍼 채우기 - 행렬 전치 금지 (row_major + raw memcpy 전제) */
    D3D11_MAPPED_SUBRESOURCE ms{};
    if (FAILED(m_pContext->Map(m_pFroxelCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
        return E_FAIL;

    FROXEL_CB* cb = reinterpret_cast<FROXEL_CB*>(ms.pData);
    cb->mCamViewInv = *m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW);
    cb->mCamProjInv = *m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ);
    cb->mShadowView = *m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW);
    cb->mShadowProj = *m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ);
    cb->vCamPos = *m_pGameInstance_Proxy->Get_CamPosition();

    if (nullptr != pDirLight)
    {
        cb->vLightDir = pDirLight->vDirection;
        cb->vLightColor = pDirLight->vDiffuse;   // 강도는 Diffuse에 포함된다고 가정
    }
    else
    {
        cb->vLightDir = _float4(0.f, -1.f, 0.f, 0.f);
        cb->vLightColor = _float4(0.f, 0.f, 0.f, 0.f);
    }

    auto Fog = [&](const _char* n) -> _float4
        {
            const _float4* p = m_pGameInstance_Proxy->Get_ShaderGlobal(n);
            return p ? *p : _float4(0.f, 0.f, 0.f, 0.f);
        };
    const _float fFogNear = 0.5f;
    _float4 vColor = Fog("g_vFogColor");

    cb->vFogScatter = _float4(vColor.x, vColor.y, vColor.z, Fog("g_fFogDensity").x);
    cb->vFogParams = _float4(fFogNear, Fog("g_fFogFar").x, Fog("g_fFogHeightFalloff").x, Fog("g_fFogBaseHeight").x);
    cb->vFogParams2 = _float4(Fog("g_fFogAnisotropy").x, Fog("g_fFogAmbient").x, m_fFogTime,
        Fog("g_fFogShadowStrength").x);
    cb->vGridParams = _float4((_float)FROXEL_W, (_float)FROXEL_H, (_float)FROXEL_D, 0.f);

    m_pContext->Unmap(m_pFroxelCB, 0);

    /* 3) Inject 디스패치 - 셰도우맵은 매니저가 t0에 바인딩 */
    m_pCSInject->Bind();
    m_pCSInject->Bind_CBV(0, m_pFroxelCB);
    m_pGameInstance_Proxy->Bind_RT_CSResource(TEXT("Target_LightDepth"), 0); // t0
    m_pContext->CSSetSamplers(0, 1, &m_pShadowSampler);                       // s0
    m_pCSInject->Bind_UAV(0, m_pScatterUAV);                                  // u0
    m_pCSInject->Dispatch((FROXEL_W + 7) / 8, (FROXEL_H + 7) / 8, (FROXEL_D + 7) / 8);
    m_pCSInject->Unbind_UAVs(0, 1); // 다음 패스에서 SRV로 읽으려면 UAV 해제 필수

    /* 4) Integrate 디스패치 - froxel 볼륨은 렌더러 소유라 직접 바인딩 */
    m_pCSIntegrate->Bind();
    m_pCSIntegrate->Bind_CBV(0, m_pFroxelCB);
    m_pCSIntegrate->Bind_SRV(1, m_pScatterSRV);       // t1
    m_pCSIntegrate->Bind_UAV(0, m_pIntegUAV);         // u0
    m_pCSIntegrate->Dispatch((FROXEL_W + 7) / 8, (FROXEL_H + 7) / 8, 1);

    /* 5) 정리 - combine 에서 m_pIntegSRV(PS) 읽으니 CS 바인딩 전부 해제 */
    m_pCSIntegrate->Unbind_UAVs(0, 1);
    ID3D11ShaderResourceView* pNullSRV[2] = { nullptr, nullptr };
    m_pContext->CSSetShaderResources(0, 2, pNullSRV); // t0(셰도우), t1(scatter)
    m_pCSIntegrate->Unbind_Shader();

    return S_OK;
}

HRESULT CRenderer::Render_Bloom()
{
    const _uint iHalfW = m_iRTWidth / 2;
    const _uint iHalfH = m_iRTHeight / 2;

    _float2 vTexel = { 1.f / iHalfW, 1.f / iHalfH };

    if (FAILED(m_pShaderPost->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;

    Change_ViewportDesc(iHalfW, iHalfH);

    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_BloomA"), nullptr, false)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_ShaderGlobals(m_pShaderPost, "g_fThreshold")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Scene_SSR"), m_pShaderPost, "g_SceneTexture")))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Begin(ETOUI(POSTPROSESS::BRIGHT))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_BloomB"), nullptr, false)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_BloomA"), m_pShaderPost, "g_BloomTexture")))
        return E_FAIL;
    _float2 dirH = { 1,0 };
    if (FAILED(m_pShaderPost->Bind_RawValue("g_vBlurDir", &dirH, sizeof(_float2))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Bind_RawValue("g_vTexelSize", &vTexel, sizeof(_float2))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Begin(ETOUI(POSTPROSESS::BLUR))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_BloomA"), nullptr, false)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_BloomB"), m_pShaderPost, "g_BloomTexture")))
        return E_FAIL;
    _float2 dirV = { 0, 1 };
    if (FAILED(m_pShaderPost->Bind_RawValue("g_vBlurDir", &dirV, sizeof(_float2))))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Begin(ETOUI(POSTPROSESS::BLUR))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    Change_ViewportDesc(Render_Width(), Render_Height());

    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Scene_SSR"), m_pShaderPost, "g_SceneTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_BloomA"), m_pShaderPost, "g_BloomTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_ShaderGlobals(m_pShaderPost, "g_fBloomIntensity")))
        return E_FAIL;
    if (FAILED(m_pShaderPost->Begin(ETOUI(POSTPROSESS::COMPSITE))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
    for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERID::NONLIGHT)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ETOUI(RENDERID::NONLIGHT)].clear();

    return S_OK;
}

HRESULT CRenderer::Render_Blend()
{
    for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERID::BLEND)])
    {
        if (nullptr != pRenderObject)
            pRenderObject->Render();

        Safe_Release(pRenderObject);
    }

    m_RenderObjects[ETOUI(RENDERID::BLEND)].clear();

    return S_OK;
}

HRESULT CRenderer::Render_UI_BACK()
{
    sort(m_RenderUIs[ETOUI(RENDERUIID::BACK)].begin(),
        m_RenderUIs[ETOUI(RENDERUIID::BACK)].end(),
        [](CUIObject* a, CUIObject* b)
        {
            return a->Get_ZOrder() > b->Get_ZOrder();
        });

    for (auto& pRenderUI : m_RenderUIs[ETOUI(RENDERUIID::BACK)])
    {
        if (nullptr != pRenderUI)
            pRenderUI->Render();

        Safe_Release(pRenderUI);
    }

    m_RenderUIs[ETOUI(RENDERUIID::BACK)].clear();

    return S_OK;
}

HRESULT CRenderer::Render_UI_MIDDLE()
{
    sort(m_RenderUIs[ETOUI(RENDERUIID::MIDDLE)].begin(),
        m_RenderUIs[ETOUI(RENDERUIID::MIDDLE)].end(),
        [](CUIObject* a, CUIObject* b)
        {
            return a->Get_ZOrder() > b->Get_ZOrder();
        });

    for (auto& pRenderUI : m_RenderUIs[ETOUI(RENDERUIID::MIDDLE)])
    {
        if (nullptr != pRenderUI)
            pRenderUI->Render();

        Safe_Release(pRenderUI);
    }

    m_RenderUIs[ETOUI(RENDERUIID::MIDDLE)].clear();

    return S_OK;
}

HRESULT CRenderer::Render_UI_FRONT()
{
    sort(m_RenderUIs[ETOUI(RENDERUIID::FRONT)].begin(),
        m_RenderUIs[ETOUI(RENDERUIID::FRONT)].end(),
        [](CUIObject* a, CUIObject* b)
        {
            return a->Get_ZOrder() > b->Get_ZOrder();
        });

    for (auto& pRenderUI : m_RenderUIs[ETOUI(RENDERUIID::FRONT)])
    {
        if (nullptr != pRenderUI)
            pRenderUI->Render();

        Safe_Release(pRenderUI);
    }

    m_RenderUIs[ETOUI(RENDERUIID::FRONT)].clear();

    return S_OK;
}

_uint CRenderer::Render_Width() const
{
    return m_pOutRTV ? m_iOutWidth : m_iRTWidth;
} 

_uint CRenderer::Render_Height() const
{
    return m_pOutRTV ? m_iOutHeight : m_iRTHeight;
}

HRESULT CRenderer::Ready_DepthStencil_Buffer()
{
    ID3D11Texture2D* pMaxDSTexture = { nullptr };

    D3D11_TEXTURE2D_DESC	MaxTextureDesc{};
    MaxTextureDesc.Width = g_iMaxWidth;
    MaxTextureDesc.Height = g_iMaxHeight;
    MaxTextureDesc.MipLevels = 1;
    MaxTextureDesc.ArraySize = 1;
    MaxTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    MaxTextureDesc.SampleDesc.Quality = 0;
    MaxTextureDesc.SampleDesc.Count = 1;

    MaxTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    MaxTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    MaxTextureDesc.CPUAccessFlags = 0;
    MaxTextureDesc.MiscFlags = 0;

    if (FAILED(m_pDevice->CreateTexture2D(&MaxTextureDesc, nullptr, &pMaxDSTexture)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateDepthStencilView(pMaxDSTexture, nullptr, &m_pMaxDSV)))
        return E_FAIL;

    Safe_Release(pMaxDSTexture);

    return S_OK;
}

HRESULT CRenderer::Ready_Froxel_Volumes()
{
    auto CreateVol = [&](ID3D11Texture3D** ppTex, ID3D11UnorderedAccessView** ppUAV,
        ID3D11ShaderResourceView** ppSRV) -> HRESULT
        {
            D3D11_TEXTURE3D_DESC td{};
            td.Width = FROXEL_W; td.Height = FROXEL_H; td.Depth = FROXEL_D;
            td.MipLevels = 1;
            td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
            if (FAILED(m_pDevice->CreateTexture3D(&td, nullptr, ppTex)))
                return E_FAIL;

            D3D11_UNORDERED_ACCESS_VIEW_DESC ud{};
            ud.Format = td.Format;
            ud.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
            ud.Texture3D.FirstWSlice = 0;
            ud.Texture3D.WSize = FROXEL_D;
            ud.Texture3D.MipSlice = 0;
            if (FAILED(m_pDevice->CreateUnorderedAccessView(*ppTex, &ud, ppUAV)))
                return E_FAIL;

            D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
            sd.Format = td.Format;
            sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            sd.Texture3D.MipLevels = 1;
            sd.Texture3D.MostDetailedMip = 0;
            if (FAILED(m_pDevice->CreateShaderResourceView(*ppTex, &sd, ppSRV)))
                return E_FAIL;

            return S_OK;
        };

    if (FAILED(CreateVol(&m_pScatterTex, &m_pScatterUAV, &m_pScatterSRV)))
        return E_FAIL;
    if (FAILED(CreateVol(&m_pIntegTex, &m_pIntegUAV, &m_pIntegSRV)))
        return E_FAIL;

    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(FROXEL_CB);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(m_pDevice->CreateBuffer(&bd, nullptr, &m_pFroxelCB)))
        return E_FAIL;

    m_pCSInject = CComputeShader::Create(m_pDevice, m_pContext,
        TEXT("../Bin/ShaderFiles/Shader_VolumetricFog.hlsl"), "CS_Inject");
    if (nullptr == m_pCSInject)
        return E_FAIL;
    m_pCSIntegrate = CComputeShader::Create(m_pDevice, m_pContext,
        TEXT("../Bin/ShaderFiles/Shader_VolumetricFog.hlsl"), "CS_Integrate");
    if (nullptr == m_pCSIntegrate)
        return E_FAIL;

    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = sampDesc.AddressV = sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0.f;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(m_pDevice->CreateSamplerState(&sampDesc, &m_pShadowSampler)))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Change_ViewportDesc(_uint iWidth, _uint iHeight)
{
    D3D11_VIEWPORT			ViewPortDesc;
    ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
    ViewPortDesc.TopLeftX = 0;
    ViewPortDesc.TopLeftY = 0;
    ViewPortDesc.Width = (_float)iWidth;
    ViewPortDesc.Height = (_float)iHeight;
    ViewPortDesc.MinDepth = 0.f;
    ViewPortDesc.MaxDepth = 1.f;

    m_pContext->RSSetViewports(1, &ViewPortDesc);

    return S_OK;
}

#ifdef _DEBUG
HRESULT CRenderer::Render_Debug()
{
    if (!m_bDebugRender) return S_FALSE;

    for (auto& pDebugCom : m_DebugComponents)
    {
        pDebugCom->Render();
        Safe_Release(pDebugCom);
    }

    m_DebugComponents.clear();

    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShaderDeferred->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_GameObjects"), m_pShaderDeferred, m_pVIBuffer);
    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_LightAcc"), m_pShaderDeferred, m_pVIBuffer);
    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_ShadowObjects"), m_pShaderDeferred, m_pVIBuffer);

    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_Scene"), m_pShaderDeferred, m_pVIBuffer);
    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_BloomA"), m_pShaderDeferred, m_pVIBuffer);
    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_BloomB"), m_pShaderDeferred, m_pVIBuffer);

    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_SSAO"), m_pShaderDeferred, m_pVIBuffer);
    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_SSAO_Blur"), m_pShaderDeferred, m_pVIBuffer);

    return S_OK;
}
#endif

CRenderer* CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRenderer* pInstance = new CRenderer(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CRenderer");
        Safe_Release(pInstance);
    }

    return pInstance;
}


void CRenderer::Free()
{
#ifdef _DEBUG
    for (auto& pDebugCom : m_DebugComponents)
    {
        Safe_Release(pDebugCom);
    }
    m_DebugComponents.clear();
#endif // _DEBUG

    __super::Free();

    for (auto& RenderObjects : m_RenderObjects)
    {
        for (auto& pRenderObject : RenderObjects)
            Safe_Release(pRenderObject);
        RenderObjects.clear();        
    }

    for (auto& RenderUIs : m_RenderUIs)
    {
        for (auto& pRenderUI : RenderUIs)
            Safe_Release(pRenderUI);
        RenderUIs.clear();
    }

    Safe_Release(m_pShadowSampler);

    Safe_Release(m_pScatterSRV);
    Safe_Release(m_pScatterUAV);
    Safe_Release(m_pScatterTex);
    Safe_Release(m_pIntegSRV);
    Safe_Release(m_pIntegUAV);
    Safe_Release(m_pIntegTex);
    Safe_Release(m_pFroxelCB);
    Safe_Release(m_pCSInject);
    Safe_Release(m_pCSIntegrate);

    Safe_Release(m_pMaxDSV);
    Safe_Release(m_pShaderPost);
    Safe_Release(m_pShaderDeferred);
    Safe_Release(m_pVIBuffer);
    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
