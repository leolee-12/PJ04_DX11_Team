#include "Panel_Toolbar.h"

#include "EditInstance.h"
#include "Level_Edit.h"
#include "GameInstance.h"

#include "imgui.h"

CPanel_Toolbar::CPanel_Toolbar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Toolbar");
}

void CPanel_Toolbar::Render()
{
    if (!Begin_Panel())
    {
        End_Panel();
        return;
    }

    CLevel_Edit* pLevel = CEditInstance::GetInstance()->Get_Level();
    if (nullptr == pLevel)
    {
        End_Panel();
        return;
    }

    Draw_FileButtons(pLevel);
    ImGui::SameLine();
    Draw_GizmoButtons();
    ImGui::SameLine();

    // --- KeyInput Toggle (엔진 DirectInput on/off) ---
    if (m_bKeyInputEnabled)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.f));
        if (ImGui::Button("KeyInput [ON]"))
        {
            m_bKeyInputEnabled = false;
            m_pGI_Proxy->Disable_InputDeveice();
        }
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.f));
        if (ImGui::Button("KeyInput [OFF]"))
        {
            m_bKeyInputEnabled = true;
            m_pGI_Proxy->Enable_InputDeveice();
        }
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    Draw_CameraButtons(pLevel);

    End_Panel();
}

void CPanel_Toolbar::Draw_FileButtons(CLevel_Edit* pLevel)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    // -- Save --
    if (ImGui::Button("Save"))
    {
        memset(m_szSaveName, 0, sizeof(m_szSaveName));
        ImGui::OpenPopup("Save Level Name");
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Save Level Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Name:");
        ImGui::InputText("##savename", m_szSaveName, BUF_SIZE);
        if (ImGui::Button("OK"))
        {
            wstring strLevelName(m_szSaveName, m_szSaveName + strlen(m_szSaveName));
            wstring strFilePath(g_strMapModelPath + strLevelName + L".JSON");
            pLevel->Save_Level(strFilePath, strLevelName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    // -- Load --
    if (ImGui::Button("Load"))
    {
        memset(m_szLoadName, 0, sizeof(m_szLoadName));
        ImGui::OpenPopup("Load Level Name");
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Load Level Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Name:");
        ImGui::InputText("##loadname", m_szLoadName, BUF_SIZE);
        if (ImGui::Button("OK"))
        {
            wstring strLevelName(m_szLoadName, m_szLoadName + strlen(m_szLoadName));
            wstring strFilePath(g_strMapModelPath + strLevelName + L".JSON");
            pLevel->Load_Level(strFilePath);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    // -- Add Layer --
    if (ImGui::Button("Add Layer"))
    {
        memset(m_szLayerName, 0, sizeof(m_szLayerName));
        ImGui::OpenPopup("Add Layer Name");
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Add Layer Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Name:");
        ImGui::InputText("##layername", m_szLayerName, BUF_SIZE);
        if (ImGui::Button("OK"))
        {
            wstring strLayerName(m_szLayerName, m_szLayerName + strlen(m_szLayerName));
            pLevel->Add_Layer(strLayerName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void CPanel_Toolbar::Draw_GizmoButtons()
{
    CEditInstance* pEI = CEditInstance::GetInstance();
    GIZMO_OP eOp = pEI->Get_GizmoOp();

    auto OpButton = [&](const char* label, GIZMO_OP op)
        {
            _bool bActive = (eOp == op);
            if (bActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
            if (ImGui::Button(label)) pEI->Set_GizmoOp(op);
            if (bActive) ImGui::PopStyleColor();
        };

    OpButton("Move", GIZMO_OP::TRANSLATE);
    ImGui::SameLine();
    OpButton("Rotate", GIZMO_OP::ROTATE);
    ImGui::SameLine();
    OpButton("Scale", GIZMO_OP::SCALE);
}

void CPanel_Toolbar::Draw_CameraButtons(CLevel_Edit* pLevel)
{
    if (ImGui::Button("Back to Edit"))
        pLevel->Back_To_Edit();

    ImGui::SameLine();

    if (ImGui::Button("Cameras"))
        ImGui::OpenPopup("CamerasPopup");

    if (ImGui::BeginPopup("CamerasPopup"))
    {
        const auto* pLayer = pLevel->Get_CameraLayer();
        if (pLayer && !pLayer->empty())
        {
            for (const auto& handle : *pLayer)
            {
                string strName(handle.strName.begin(), handle.strName.end());
                if (ImGui::Selectable(strName.c_str()))
                {
                    pLevel->Preview_Camera(handle.pObject);
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        else
        {
            ImGui::TextDisabled("(No cameras)");
        }
        ImGui::EndPopup();
    }
}

CPanel_Toolbar* CPanel_Toolbar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Toolbar(pDevice, pContext);
}

void CPanel_Toolbar::Free()
{
    __super::Free();
}
