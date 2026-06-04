#include "ImGui_Manager.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

CImGui_Manager::CImGui_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice), m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CImGui_Manager::Initialize()
{
	if (m_bContextInitialized)
		return S_OK;

	if (nullptr == m_pDevice || nullptr == m_pContext || nullptr == g_hWnd)
		return E_FAIL;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_bContextInitialized = true;

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	if (!ImGui_ImplWin32_Init(g_hWnd))
		return E_FAIL;
	m_bWin32Initialized = true;

	if (!ImGui_ImplDX11_Init(m_pDevice, m_pContext))
		return E_FAIL;
	m_bDX11Initialized = true;

	return S_OK;
}

void CImGui_Manager::Begin_Frame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void CImGui_Manager::Draw_LoadingOverlay(_float fProgress)
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
		ImGui::ProgressBar(fProgress, ImVec2(320.f, 0.f));
	}
	ImGui::End();
}

void CImGui_Manager::Render()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

CImGui_Manager* CImGui_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CImGui_Manager* pInstance = new CImGui_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CImGui_Manager");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CImGui_Manager::Free()
{
	if (m_bContextInitialized && ImGui::GetCurrentContext())
	{
		if ((m_bWin32Initialized || m_bDX11Initialized) && (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
			ImGui::DestroyPlatformWindows();

		if (m_bDX11Initialized)
			ImGui_ImplDX11_Shutdown();

		if (m_bWin32Initialized)
			ImGui_ImplWin32_Shutdown();

		ImGui::DestroyContext();
	}

	m_bDX11Initialized = false;
	m_bWin32Initialized = false;
	m_bContextInitialized = false;

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	__super::Free();
}
