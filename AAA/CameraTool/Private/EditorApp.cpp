#include "EditorApp.h"

#include "GameInstance.h"
#include "ImGui_Manager.h"
#include "Level_Edit.h"

#include "Loader_Prototype.h"
#include "GameObject_Factory.h"
#include "Effect_Loader.h"
#include "Projectile_Manager.h"

CEditorApp::CEditorApp()
{
}

HRESULT CEditorApp::Initialize()
{
    ENGINE_DESC         EngineDesc{};
	EngineDesc.hInstance = g_hInstance;
    EngineDesc.hWnd = g_hWndEditor;
    EngineDesc.eWinMode = WINMODE::WIN;
    EngineDesc.iViewportWidth = g_iWinSizeX;
    EngineDesc.iViewportHeight = g_iWinSizeY;
    EngineDesc.iNumLevels = max(ETOUI(LEVEL::END), ETOUI(EDIT_LEVEL::END));

    if (FAILED(CGameInstance::Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
    {
        MSG_BOX("Failed to Initialize : Engine");
        return E_FAIL;
    }

    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    m_pGameInstance_Proxy->Disable_InputDeveice();
    m_pGameInstance_Proxy->Set_EditMode(true);

    Ready_EditRTV();
    m_pGameInstance_Proxy->Bind_RenderTarget(m_pRTV, m_pDSV, g_iWinSizeX, g_iWinSizeY);

    CGameObject_Factory::GetInstance()->RegisterAll();

    if (FAILED(Ready_SharedResources()))
        return E_FAIL;

    if (FAILED(Load_Fonts(m_pGameInstance_Proxy)))
        return E_FAIL;

    m_pImGui_Manager = CImGui_Manager::GetInstance();
    Safe_AddRef(m_pImGui_Manager);

    CLevel_Edit* pPreLevel = CLevel_Edit::Create(m_pDevice, m_pContext);
    if (nullptr == pPreLevel)
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOI(EDIT_LEVEL::EDIT), pPreLevel)))
        return E_FAIL;

    m_pLevel_Edit = pPreLevel;
    m_pImGui_Manager->ImGui_Initialize(&m_pDevice, &m_pContext, pPreLevel, &m_pSRV, &m_pSRV2);

    return S_OK;
}

void CEditorApp::Update(_float fTimeDelta, _float fRawTimeDelta)
{
    m_pGameInstance_Proxy->Update_Engine(fTimeDelta, fRawTimeDelta);
}

HRESULT CEditorApp::Render()
{
    bool bPreviewOn = (m_pLevel_Edit && m_pLevel_Edit->Is_Preview());
    m_bPreviewFrame = bPreviewOn ? !m_bPreviewFrame : false;

    if (bPreviewOn && m_bPreviewFrame)
    {
        // ---- PREVIEW FRAME -> RTV2 (편집 RTV는 안 건드림 = 직전 편집 이미지 유지) ----
        _float4x4 editView = *m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
        _float4x4 editProj = *m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);

        _float4x4 pView, pProj;
        m_pLevel_Edit->Get_PreviewViewProj(&pView, &pProj);
        m_pGameInstance_Proxy->Set_Transform(D3DTS::VIEW, PROJ_TYPE::PERSPEC, pView);
        m_pGameInstance_Proxy->Set_Transform(D3DTS::PROJ, PROJ_TYPE::PERSPEC, pProj);

        const _float clr[4] = { 0.18f, 0.18f, 0.22f, 1.f };
        m_pContext->ClearRenderTargetView(m_pRTV2, clr);
        m_pContext->ClearDepthStencilView(m_pDSV2, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
        m_pGameInstance_Proxy->Bind_RenderTarget(m_pRTV2, m_pDSV2, g_iWinSizeX, g_iWinSizeY);

        if (FAILED(m_pGameInstance_Proxy->Draw())) return E_FAIL;

        // 메인 타깃/매트릭스 복구 (다음 편집 프레임 + ImGui 오버레이용)
        m_pGameInstance_Proxy->Bind_RenderTarget(m_pRTV, m_pDSV, g_iWinSizeX, g_iWinSizeY);
        m_pGameInstance_Proxy->Set_Transform(D3DTS::VIEW, PROJ_TYPE::PERSPEC, editView);
        m_pGameInstance_Proxy->Set_Transform(D3DTS::PROJ, PROJ_TYPE::PERSPEC, editProj);
    }
    else
    {
        // ---- EDIT FRAME -> RTV (원래 working 경로 그대로) ----
        Editor_BeginDraw();
        if (FAILED(m_pGameInstance_Proxy->Draw())) return E_FAIL;
    }

    if (FAILED(m_pGameInstance_Proxy->Begin_Draw())) return E_FAIL;
    m_pImGui_Manager->ImGui_Render();
    if (FAILED(m_pGameInstance_Proxy->End_Draw())) return E_FAIL;
    return S_OK;
}

HRESULT CEditorApp::Start_Level()
{
    CLevel* pPreLevel = CLevel_Edit::Create(m_pDevice, m_pContext);
    if (nullptr == pPreLevel)
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOI(EDIT_LEVEL::EDIT), pPreLevel)))
        return E_FAIL;

    return S_OK;
}

HRESULT CEditorApp::Ready_SharedResources()
{
    if (FAILED(Ready_Prototype_SharedResources(m_pGameInstance_Proxy, m_pDevice, m_pContext)))
    {
        MSG_BOX("Load Failed : SharedResources");
        return E_FAIL;
    }

    if (FAILED(Ready_Prototype_Shaders(m_pGameInstance_Proxy, m_pDevice, m_pContext)))
    {
        MSG_BOX("Load Failed : Shaders");
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CEditorApp::Ready_EditRTV()
{
    if (FAILED(Make_OffscreenRTV(&m_pRTV, &m_pSRV, &m_pDSV)))  return E_FAIL;
    if (FAILED(Make_OffscreenRTV(&m_pRTV2, &m_pSRV2, &m_pDSV2))) return E_FAIL;
    return S_OK;
}

HRESULT CEditorApp::Make_OffscreenRTV(ID3D11RenderTargetView** ppRTV, ID3D11ShaderResourceView** ppSRV, ID3D11DepthStencilView** ppDSV)
{
    if (nullptr == m_pDevice) return E_FAIL;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = g_iWinSizeX; desc.Height = g_iWinSizeY; desc.MipLevels = 1; desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1; desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    ID3D11Texture2D* pTex = nullptr;
    if (FAILED(m_pDevice->CreateTexture2D(&desc, nullptr, &pTex))) return E_FAIL;
    if (FAILED(m_pDevice->CreateRenderTargetView(pTex, nullptr, ppRTV))) return E_FAIL;
    if (FAILED(m_pDevice->CreateShaderResourceView(pTex, nullptr, ppSRV))) return E_FAIL;
    Safe_Release(pTex);

    D3D11_TEXTURE2D_DESC dd{};
    dd.Width = g_iWinSizeX; dd.Height = g_iWinSizeY; dd.MipLevels = 1; dd.ArraySize = 1;
    dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; dd.SampleDesc.Count = 1; dd.Usage = D3D11_USAGE_DEFAULT;
    dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ID3D11Texture2D* pDS = nullptr;
    if (FAILED(m_pDevice->CreateTexture2D(&dd, nullptr, &pDS))) return E_FAIL;
    if (FAILED(m_pDevice->CreateDepthStencilView(pDS, nullptr, ppDSV))) return E_FAIL;
    Safe_Release(pDS);
    return S_OK;
}

void CEditorApp::Editor_BeginDraw()
{
    D3D11_VIEWPORT ViewPortDesc;
    ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
    ViewPortDesc.TopLeftX = 0;
    ViewPortDesc.TopLeftY = 0;
    ViewPortDesc.Width  = (_float)g_iWinSizeX;
    ViewPortDesc.Height = (_float)g_iWinSizeY;
    ViewPortDesc.MinDepth = 0.f;
    ViewPortDesc.MaxDepth = 1.f;

    m_pContext->RSSetViewports(1, &ViewPortDesc);

    _float clearColor[4] = { 0.35f, 0.35f, 0.35f, 1.f };
    m_pContext->ClearRenderTargetView(m_pRTV, clearColor);
    m_pContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

CEditorApp* CEditorApp::Create()
{
    CEditorApp* pInstance = new CEditorApp();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CEditorApp");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEditorApp::Free()
{
    __super::Free();

    CEffect_Loader::DestroyInstance();
    CProjectile_Manager::DestroyInstance();

    Safe_Release(m_pRTV);
    Safe_Release(m_pSRV);
    Safe_Release(m_pDSV);

    Safe_Release(m_pRTV2);
    Safe_Release(m_pSRV2);
    Safe_Release(m_pDSV2);

    Safe_Release(m_pImGui_Manager);
    CImGui_Manager::DestroyInstance();

    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);

    CGameInstance::DestroyInstance();

    CGameObject_Factory::DestroyInstance();
}
