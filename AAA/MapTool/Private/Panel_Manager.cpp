#include "Panel_Manager.h"

#include "Panel_Toolbar.h"
#include "Panel_Hierarchy.h"
#include "Panel_Palette.h"
#include "Panel_Viewport.h"
#include "Panel_Inspector.h"
#include "Panel_Profiler.h"

#include "imgui.h"
#include "imgui_internal.h"

CPanel_Manager::CPanel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice), m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CPanel_Manager::Initialize()
{
	if (FAILED(Add_Panel(L"Toolbar", CPanel_Toolbar::Create(m_pDevice, m_pContext))))   return E_FAIL;
	if (FAILED(Add_Panel(L"Hierarchy", CPanel_Hierarchy::Create(m_pDevice, m_pContext)))) return E_FAIL;
	if (FAILED(Add_Panel(L"Palette", CPanel_Palette::Create(m_pDevice, m_pContext))))   return E_FAIL;
	if (FAILED(Add_Panel(L"Viewport", CPanel_Viewport::Create(m_pDevice, m_pContext))))  return E_FAIL;
	if (FAILED(Add_Panel(L"Inspector", CPanel_Inspector::Create(m_pDevice, m_pContext)))) return E_FAIL;
	if (FAILED(Add_Panel(L"Profiler", CPanel_Profiler::Create(m_pDevice, m_pContext))))  return E_FAIL;

	return S_OK;
}

HRESULT CPanel_Manager::Add_Panel(const _wstring& strPanelTag, CPanel* pPanel)
{
	if (nullptr == pPanel)
		return E_FAIL;

	if (m_Panels.find(strPanelTag) != m_Panels.end())
	{
		Safe_Release(pPanel);
		return E_FAIL;
	}

	if (FAILED(pPanel->Initialize(this)))
	{
		Safe_Release(pPanel);
		return E_FAIL;
	}

	m_Panels.emplace(strPanelTag, pPanel);
	return S_OK;
}

void CPanel_Manager::Update(_float fTimeDelta)
{
	for (auto& [tag, pPanel] : m_Panels)
		if (pPanel->Is_Open())
			pPanel->Update(fTimeDelta);
}

void CPanel_Manager::Render()
{
	Render_DockSpace();

	for (auto& [tag, pPanel] : m_Panels)
		if (pPanel->Is_Open())
			pPanel->Render();
}

void CPanel_Manager::Render_DockSpace()
{
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->WorkPos);
	ImGui::SetNextWindowSize(vp->WorkSize);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpaceHost", nullptr, flags);
	ImGui::PopStyleVar(3);

	// -- 메뉴바: 패널 열기/닫기 토글 (docs ImGui_Manager 의 Window 메뉴) --
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Window"))
		{
			for (auto& [tag, pPanel] : m_Panels)
				ImGui::MenuItem(pPanel->Get_Name(), nullptr, pPanel->Get_OpenPtr());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	if (!m_bLayoutBuilt && ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
	{
		m_bLayoutBuilt = true;

		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, vp->WorkSize);

		ImGuiID mainArea, leftColumn, centerColumn, rightColumn;
		ImGuiID hierarchyNode, paletteNode, viewportNode, bottomToolsNode;

		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.36f, &rightColumn, &mainArea);
		ImGui::DockBuilderSplitNode(mainArea, ImGuiDir_Left, 0.18f, &leftColumn, &centerColumn);
		ImGui::DockBuilderSplitNode(leftColumn, ImGuiDir_Down, 0.45f, &paletteNode, &hierarchyNode);
		ImGui::DockBuilderSplitNode(centerColumn, ImGuiDir_Down, 0.34f, &bottomToolsNode, &viewportNode);

		ImGui::DockBuilderDockWindow("Hierarchy", hierarchyNode);
		ImGui::DockBuilderDockWindow("Palette", paletteNode);
		ImGui::DockBuilderDockWindow("Viewport", viewportNode);
		ImGui::DockBuilderDockWindow("Toolbar", bottomToolsNode);
		ImGui::DockBuilderDockWindow("Inspector", rightColumn);
		ImGui::DockBuilderDockWindow("Profiler", rightColumn);

		if (ImGuiDockNode* pRightNode = ImGui::DockBuilderGetNode(rightColumn))
			pRightNode->SelectedTabId = ImHashStr("Inspector");

		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);
	ImGui::End();
}

CPanel_Manager* CPanel_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPanel_Manager* pInstance = new CPanel_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CPanel_Manager");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPanel_Manager::Free()
{
	__super::Free();

	for (auto& [tag, pPanel] : m_Panels)
		Safe_Release(pPanel);
	m_Panels.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
