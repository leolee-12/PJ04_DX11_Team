#include "Renderer.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "UIObject.h"

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
    _uint iWidth = static_cast<_uint>(m_pGameInstance_Proxy->Get_WindowWidth());
    _uint iHeight = static_cast<_uint>(m_pGameInstance_Proxy->Get_WindowHeight());

    /* 렌더타겟들을 만든다. */
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Diffuse"), iWidth, iHeight, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Normal"), iWidth, iHeight, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 1.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Depth"), iWidth, iHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_MRA"), iWidth, iHeight, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 1.f, 1.f, 0.f))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_RenderTarget(TEXT("Target_Light"), iWidth, iHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
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

    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Light"))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_MRT(TEXT("MRT_ShadowObjects"), TEXT("Target_LightDepth"))))
        return E_FAIL;


    m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
    if (nullptr == m_pShader)
        return E_FAIL;

    m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
    if (nullptr == m_pVIBuffer)
        return E_FAIL;

    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(static_cast<_float>(iWidth), static_cast<_float>(iHeight), 1.f) * XMMatrixTranslation(0.f, 0.f, 0.1f));

#ifdef _DEBUG
    //if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Diffuse"), 150.f, 100.f, 300.f, 200.f)))
    //    return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_MRA"), 150.f, 100.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Diffuse"), 150.f, 300.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Depth"), 150.f, 500.f, 300.f, 200.f)))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Normal"), 150.f, 700.f, 300.f, 200.f)))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Ready_RT_Debug(TEXT("Target_Light"), 450.f, 100.f, 300.f, 200.f)))
        return E_FAIL;
#endif

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
    if (FAILED(Render_Priority()))
        return E_FAIL;
    if (FAILED(Render_Shadow()))
        return E_FAIL;
    if (FAILED(Render_NonBlend()))
        return E_FAIL;
    if (FAILED(Render_Lights()))
        return E_FAIL;
    if (FAILED(Render_Combined()))
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
    m_DebugComponents.push_back(pComponent);
    Safe_AddRef(pComponent);
}
#endif

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

    Change_ViewportDesc((_uint)m_pGameInstance_Proxy->Get_WindowWidth(), (_uint)m_pGameInstance_Proxy->Get_WindowHeight());

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

HRESULT CRenderer::Render_Lights()
{
    if (FAILED(m_pGameInstance_Proxy->Begin_MRT(TEXT("MRT_LightAcc"))))
        return E_FAIL;

    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MRA"), m_pShader, "g_MRATexture")))
        return E_FAIL;

    if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance_Proxy->Get_CamPosition(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Render_Light(m_pShader, m_pVIBuffer)))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->End_MRT()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderer::Render_Combined()
{
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Light"), m_pShader, "g_LightTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MRA"), m_pShader, "g_MRATexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
        return E_FAIL;
    if(FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_LightDepth"), m_pShader, "g_LightDepthTexture")))
        return E_FAIL;

    if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ShadowLightViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ShadowLightProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pShader->Begin(ETOUI(DEFERRED::COMBINED))))
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

HRESULT CRenderer::Ready_DepthStencil_Buffer()
{
    ID3D11Texture2D* pDepthStencilTexture = { nullptr };

    D3D11_TEXTURE2D_DESC	TextureDesc{};
    TextureDesc.Width = g_iMaxWidth;
    TextureDesc.Height = g_iMaxHeight;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;

    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;

    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pMaxDSV)))
        return E_FAIL;

    Safe_Release(pDepthStencilTexture);

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
    for (auto& pDebugCom : m_DebugComponents)
    {
        pDebugCom->Render();
        Safe_Release(pDebugCom);
    }

    m_DebugComponents.clear();

    if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::ORTHO))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO))))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_GameObjects"), m_pShader, m_pVIBuffer);
    m_pGameInstance_Proxy->Render_RT_Debug(TEXT("MRT_LightAcc"), m_pShader, m_pVIBuffer);

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

    Safe_Release(m_pMaxDSV);
    Safe_Release(m_pShader);
    Safe_Release(m_pVIBuffer);
    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
