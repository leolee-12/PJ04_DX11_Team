#include "ToolApp.h"

#include "EditInstance.h"
#include "Level_Loading.h"

#include "GameObject_Factory.h"
#include "GameInstance.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

CToolApp::CToolApp()
{
}

HRESULT CToolApp::Initialize()
{
	if (FAILED(Ready_Engine()))
		return E_FAIL;

	if (FAILED(Ready_EditRTV()))
		return E_FAIL;

	if (FAILED(Init_ImGui()))
		return E_FAIL;

	// 공유 상태 허브 (씬 SRV 등록)
	m_pEditInstance = CEditInstance::GetInstance();
	Safe_AddRef(m_pEditInstance);
	m_pEditInstance->Set_SceneSRV(m_pSRV);

	if (FAILED(m_pEditInstance->Initialize(m_pDevice, m_pContext)))
		return E_FAIL;

	// 팩토리 레시피 등록(배치 팔레트/인스펙터 자동 연동)
	CGameObject_Factory::GetInstance()->RegisterAll();

	// 로딩 레벨 진입 → 워커가 공용 리소스/셰이더/폰트 적재 → 완료 시 EDIT 자동 전환
	CLevel_Loading* pLoading = CLevel_Loading::Create(m_pDevice, m_pContext, TOOL_LEVEL::EDIT);
	if (nullptr == pLoading)
		return E_FAIL;

	if (FAILED(m_pGI_Proxy->Change_Level(ETOI(TOOL_LEVEL::LOADING), pLoading)))
		return E_FAIL;

	return S_OK;
}

void CToolApp::Update(_float fTimeDelta)
{
	m_pGI_Proxy->Update_Engine(fTimeDelta);

	if (!m_pEditInstance->Is_Loading())
		m_pEditInstance->Update_Panels(fTimeDelta);
}

HRESULT CToolApp::Render()
{
	// 1) 엔진 씬을 오프스크린 RTV에 렌더
	Editor_BeginDraw();

	if (FAILED(m_pGI_Proxy->Draw()))
		return E_FAIL;

	// 2) ImGui 프레임 (로딩 중에는 오버레이만, 그 외에는 패널)
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (m_pEditInstance->Is_Loading())
		Draw_LoadingOverlay();
	else
		m_pEditInstance->Render_Panels();

	ImGui::Render();

	// 3) 백버퍼에 ImGui 출력
	if (FAILED(m_pGI_Proxy->Begin_Draw()))
		return E_FAIL;

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

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
	return S_OK;
}

HRESULT CToolApp::Init_ImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;   // 도킹 레이아웃은 Panel_Manager가 DockBuilder로 구성

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	return S_OK;
}

void CToolApp::Draw_LoadingOverlay()
{
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowBgAlpha(0.85f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoDocking;

	if (ImGui::Begin("##LoadingOverlay", nullptr, flags))
	{
		ImGui::Text("Loading Map Tool...");
		ImGui::ProgressBar(m_pEditInstance->Get_LoadProgress(), ImVec2(320.f, 0.f));
	}
	ImGui::End();
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
	m_pContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
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

	Safe_Release(m_pRTV);
	Safe_Release(m_pSRV);
	Safe_Release(m_pDSV);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Safe_Release(m_pGI_Proxy);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	CGameInstance::DestroyInstance();
	CGameObject_Factory::DestroyInstance();

	__super::Free();
}
