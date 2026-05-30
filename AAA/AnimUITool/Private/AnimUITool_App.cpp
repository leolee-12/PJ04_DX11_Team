#include "AnimUITool_App.h"
#include "GameInstance.h"
#include "Panel_Manager.h"
#include "Panel_Viewport.h"

#include "Level_Tool.h"

// ImGui Çì´õ
#include "imgui.h"
#include "imgui_internal.h"          
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

CAnimUITool_App::CAnimUITool_App()
{
}

HRESULT CAnimUITool_App::Initialize()
{
	if (FAILED(Init_Engine()))
		return E_FAIL;

	if (FAILED(Init_ImGui()))
		return E_FAIL;

	m_pPanel_Manager = CPanel_Manager::Create(m_pDevice, m_pContext);
	if (nullptr == m_pPanel_Manager) return E_FAIL;

	if (FAILED(Ready_EditRTV()))
	{
		Log_Error("Failed to ready viewport RT.");
		return E_FAIL;
	}

	if (auto* pViewport = dynamic_cast<CPanel_Viewport*>(m_pPanel_Manager->Get_Panel(L"Viewport")))
	{
		pViewport->Set_SRV(m_pSRV);
		m_pViewportPanel = pViewport;
	}

	CLevel_Tool* pLevel = CLevel_Tool::Create(m_pDevice, m_pContext);
	if (nullptr == pLevel) 
	{ 
		Log_Error("Failed to create Tool level.");
		return E_FAIL;
	}

	if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOUI(TOOL_LEVEL::EDIT), pLevel)))
		return E_FAIL;

	m_pToolLevel = pLevel;

		Log_Info("AnimUITool initialized.");

		return S_OK;

}

HRESULT CAnimUITool_App::Init_Engine()
{
	ENGINE_DESC EngineDesc{};
	EngineDesc.hInstance = g_hInstance;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.eWinMode = WINMODE::WIN;
	EngineDesc.iViewportWidth = g_iWinSizeX;
	EngineDesc.iViewportHeight = g_iWinSizeY;
	EngineDesc.iNumLevels = ETOUI(TOOL_LEVEL::END);

	if (FAILED(CGameInstance::Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
	{
		MSG_BOX("Failed to Initialize : Engine");
		return E_FAIL;
	}

	m_pGameInstance_Proxy = CGameInstance::GetProxy();
	return S_OK;
}

HRESULT CAnimUITool_App::Init_ImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = "AnimUITool_imgui.ini";

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 15.0f, 
									nullptr, io.Fonts->GetGlyphRangesKorean());

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = { 0.0f };
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	return S_OK;
}

void CAnimUITool_App::Update(_float fTimeDelta)
{
	if (m_pToolLevel && m_pViewportPanel)
		m_pToolLevel->Set_CameraActive(m_pViewportPanel->Is_Hovered());

	m_pGameInstance_Proxy->Update_Engine(fTimeDelta); 
	m_pPanel_Manager->Update(fTimeDelta);
}

HRESULT CAnimUITool_App::Render()
{
	Editor_BeginDraw();                              

	if (FAILED(m_pGameInstance_Proxy->Draw()))       
		return E_FAIL;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_pPanel_Manager->Render();

	ImGui::Render();

	if (FAILED(m_pGameInstance_Proxy->Begin_Draw())) 
		return E_FAIL;

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	if (FAILED(m_pGameInstance_Proxy->End_Draw()))
		return E_FAIL;

	return S_OK;
}

void CAnimUITool_App::Render_DockSpace()
{
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->WorkPos);
	ImGui::SetNextWindowSize(vp->WorkSize);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpaceHost", nullptr, flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
	{
		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, vp->WorkSize);

		ImGuiID left, center, right, bottom;
		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.18f, &left, &center);
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.22f, &right, &center);
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, &bottom, &center);

		ImGui::DockBuilderDockWindow("Hierarchy", left);
		ImGui::DockBuilderDockWindow("Viewport", center);
		ImGui::DockBuilderDockWindow("Inspector", right);
		ImGui::DockBuilderDockWindow("Console", bottom);
		ImGui::DockBuilderFinish(dockspace_id);
	}

	//ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);
	ImGui::End();
}

HRESULT CAnimUITool_App::Ready_EditRTV()
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

void CAnimUITool_App::Editor_BeginDraw()
{
	D3D11_VIEWPORT vp{};
	vp.TopLeftX = 0; vp.TopLeftY = 0;
	vp.Width = (_float)m_iViewportWidth;
	vp.Height = (_float)m_iViewportHeight;
	vp.MinDepth = 0.f; vp.MaxDepth = 1.f;
	m_pContext->RSSetViewports(1, &vp);

	_float clearColor[4] = { 0.35f, 0.35f, 0.35f, 1.f };
	m_pContext->ClearRenderTargetView(m_pRTV, clearColor);
	m_pContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_pContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
}

CAnimUITool_App* CAnimUITool_App::Create()
{
	CAnimUITool_App* pInstance = new CAnimUITool_App();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CAnimUITool_App");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimUITool_App::Free()
{
	__super::Free();

	Safe_Release(m_pPanel_Manager);

	Safe_Release(m_pRTV);
	Safe_Release(m_pSRV);
	Safe_Release(m_pDSV);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Safe_Release(m_pGameInstance_Proxy);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	CGameInstance::DestroyInstance();
}
