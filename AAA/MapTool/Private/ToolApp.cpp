#include "ToolApp.h"
#include "EditInstance.h"
#include "Level_Loading.h"

#include "GameObject_Factory.h"
#include "Singleton_Destroyer.h"

#include "GameInstance.h"

CToolApp::CToolApp()
{
}

HRESULT CToolApp::Initialize()
{
	if (FAILED(Ready_Engine()))
		return E_FAIL;

	if (FAILED(Ready_EditRTV()))
		return E_FAIL;

	m_pGI_Proxy->Bind_RenderTarget(m_pRTV, m_pDSV, m_iViewportWidth, m_iViewportHeight);

	// Shared scene SRV for the editor panels.
	m_pEditInstance = CEditInstance::GetInstance();
	Safe_AddRef(m_pEditInstance);
	m_pEditInstance->Set_SceneSRV(m_pSRV);

	if (FAILED(m_pEditInstance->Initialize(m_pDevice, m_pContext)))
		return E_FAIL;

	// Register factory recipes for palette and inspector integration.
	CGameObject_Factory::GetInstance()->RegisterAll();

	// Enter loading and switch to EDIT after shared resources are ready.
	CLevel_Loading* pLoading = CLevel_Loading::Create(m_pDevice, m_pContext, TOOL_LEVEL::EDIT);
	if (nullptr == pLoading)
		return E_FAIL;

	if (FAILED(m_pGI_Proxy->Change_Level(ETOI(TOOL_LEVEL::LOADING), pLoading)))
		return E_FAIL;

	return S_OK;
}

void CToolApp::Update(_float fTimeDelta, _float fRawTimeDelta)
{
	m_pGI_Proxy->Update_Engine(fTimeDelta, fRawTimeDelta);

	if (!m_pEditInstance->Is_Loading())
		m_pEditInstance->Update_Panels(fTimeDelta);
}

HRESULT CToolApp::Render()
{
	// 1) Render the engine scene into the offscreen RTV.
	Editor_BeginDraw();

	if (FAILED(m_pGI_Proxy->Draw()))
		return E_FAIL;

	// 2) Build the ImGui frame and panels.
	m_pEditInstance->ImGui_BeginFrame();
	m_pEditInstance->Render_UI();

	// 3) Present ImGui on the backbuffer.
	if (FAILED(m_pGI_Proxy->Begin_Draw()))
		return E_FAIL;

	m_pEditInstance->ImGui_Render();

	if (FAILED(m_pGI_Proxy->End_Draw()))
		return E_FAIL;

	return S_OK;
}

HRESULT CToolApp::Ready_Engine()
{
	ENGINE_DESC EngineDesc{};
	EngineDesc.hInstance		= g_hInstance;
	EngineDesc.hWnd				= g_hWnd;
	EngineDesc.eWinMode			= WINMODE::WIN;
	EngineDesc.iViewportWidth	= g_iWinSizeX;
	EngineDesc.iViewportHeight	= g_iWinSizeY;
	EngineDesc.iNumLevels		= max(ETOUI(LEVEL::END), ETOUI(TOOL_LEVEL::END));

	if (FAILED(CGameInstance::Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
	{
		MSG_BOX("Failed to Initialize : Engine");
		return E_FAIL;
	}

	m_pGI_Proxy = CGameInstance::GetProxy();
	if (nullptr == m_pGI_Proxy)
		return E_FAIL;

	m_pGI_Proxy->Set_EditMode(true);
	return S_OK;
}

HRESULT CToolApp::Ready_EditRTV()
{
	if (nullptr == m_pDevice)
		return E_FAIL;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_iViewportWidth;
	desc.Height = m_iViewportHeight;
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
	dsDesc.Width = m_iViewportWidth;
	dsDesc.Height = m_iViewportHeight;
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

void CToolApp::Editor_BeginDraw()
{
	D3D11_VIEWPORT vp{};
	vp.TopLeftX	= 0;
	vp.TopLeftY	= 0;
	vp.Width	= static_cast<_float>(m_iViewportWidth);
	vp.Height	= static_cast<_float>(m_iViewportHeight);
	vp.MinDepth	= 0.f;
	vp.MaxDepth	= 1.f;
	m_pContext->RSSetViewports(1, &vp);

	_float clearColor[4] = { 0.35f, 0.35f, 0.35f, 1.f };
	m_pContext->ClearRenderTargetView(m_pRTV, clearColor);
	m_pContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

CToolApp* CToolApp::Create()
{
	CToolApp* pInstance = new CToolApp();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CToolApp");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CToolApp::Free()
{
	Safe_Release(m_pEditInstance);
	CEditInstance::DestroyInstance();

	CSingleton_Destroyer::Destroy_GameContent_Singletons();
	Safe_Release(m_pRTV);
	Safe_Release(m_pSRV);
	Safe_Release(m_pDSV);

	Safe_Release(m_pGI_Proxy);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	CGameInstance::DestroyInstance();

	__super::Free();
}
