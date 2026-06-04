#include "EditorApp.h"

#include "GameInstance.h"
#include "ImGui_Manager.h"
#include "Level_Edit.h"
#include "MapDescriptor.h"

#include "Loader_Prototype.h"
#include "GameObject_Factory.h"

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

    

    m_pImGui_Manager = CImGui_Manager::GetInstance();
    Safe_AddRef(m_pImGui_Manager);

    CLevel_Edit* pPreLevel = CLevel_Edit::Create(m_pDevice, m_pContext);
    if (nullptr == pPreLevel)
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOI(EDIT_LEVEL::EDIT), pPreLevel)))
        return E_FAIL;

    m_pImGui_Manager->ImGui_Initialize(&m_pDevice, &m_pContext, pPreLevel, &m_pSRV);

    if (FAILED(Ready_SharedResources()))
        return E_FAIL;

    CGameObject_Factory::GetInstance()->RegisterAll();

    if (FAILED(Load_Fonts(m_pGameInstance_Proxy)))
        return E_FAIL;


    return S_OK;
}

void CEditorApp::Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Update_Engine(fTimeDelta);
}

HRESULT CEditorApp::Render()
{
    Editor_BeginDraw();
    if (FAILED(m_pGameInstance_Proxy->Draw()))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Begin_Draw()))
        return E_FAIL;
    m_pImGui_Manager->ImGui_Render();
    if (FAILED(m_pGameInstance_Proxy->End_Draw()))
        return E_FAIL;

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
    if (nullptr == m_pDevice)
        return E_FAIL;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = g_iWinSizeX;
    desc.Height = g_iWinSizeY;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* pTex = nullptr;
    if (FAILED(m_pDevice->CreateTexture2D(&desc, nullptr, &pTex)))
        return E_FAIL;
    if (FAILED(m_pDevice->CreateRenderTargetView(pTex, nullptr, &m_pRTV)))
        return E_FAIL;
    if (FAILED(m_pDevice->CreateShaderResourceView(pTex, nullptr, &m_pSRV)))
        return E_FAIL;
    Safe_Release(pTex);

    D3D11_TEXTURE2D_DESC dsDesc{};
    dsDesc.Width = g_iWinSizeX;
    dsDesc.Height = g_iWinSizeY;
    dsDesc.MipLevels = 1;
    dsDesc.ArraySize = 1;
    dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsDesc.SampleDesc.Count = 1;
    dsDesc.Usage = D3D11_USAGE_DEFAULT;
    dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* pDSTex = nullptr;
    if (FAILED(m_pDevice->CreateTexture2D(&dsDesc, nullptr, &pDSTex)))
        return E_FAIL;
    if (FAILED(m_pDevice->CreateDepthStencilView(pDSTex, nullptr, &m_pDSV)))
        return E_FAIL;
    Safe_Release(pDSTex);
    
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
    Safe_Release(m_pRTV);
    Safe_Release(m_pSRV);
    Safe_Release(m_pDSV);

    Safe_Release(m_pImGui_Manager);
    CImGui_Manager::DestroyInstance();

    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);

    CMapDescriptor::DestroyInstance();
    CGameInstance::DestroyInstance();

    CGameObject_Factory::DestroyInstance();
}
