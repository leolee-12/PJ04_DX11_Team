#include "Panel_Manager.h"
#include "Panel.h"
#include "GameObject.h"

#include "Panel_Hierarchy.h"
#include "Panel_Viewport.h"
#include "Panel_Inspector.h"
#include "Panel_Console.h"

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
    if (FAILED(Add_Panel(L"Hierarchy", CPanel_Hierarchy::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Viewport", CPanel_Viewport::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Inspector", CPanel_Inspector::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Console", CPanel_Console::Create(m_pDevice, m_pContext))))
        return E_FAIL;
    return S_OK;
}

HRESULT CPanel_Manager::Add_Panel(const _wstring& strPanelTag, CPanel* pPanel)
{
    if (nullptr == pPanel)                              
        return E_FAIL;

    if (m_Panels.find(strPanelTag) != m_Panels.end())   
        return E_FAIL;

    if (FAILED(pPanel->Initialize(this)))               
        return E_FAIL;

    m_Panels.emplace(strPanelTag, pPanel);
    return S_OK;
}

CPanel* CPanel_Manager::Get_Panel(const _wstring& strPanelTag)
{
    auto it = m_Panels.find(strPanelTag);
    return (it == m_Panels.end()) ? nullptr : it->second;
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

void CPanel_Manager::Set_Selected(Engine::CGameObject* pObject)
{
    if (m_pSelected == pObject)
        return;

    Safe_Release(m_pSelected);
    m_pSelected = pObject;
    Safe_AddRef(m_pSelected);          
}

void CPanel_Manager::Clear_Selected()
{
    Safe_Release(m_pSelected);
    m_pSelected = nullptr;
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

    Safe_Release(m_pSelected);

    for (auto& [tag, pPanel] : m_Panels)
        Safe_Release(pPanel);
    m_Panels.clear();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}