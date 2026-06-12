#include "Panel_Toolbar.h"

#include "EditInstance.h"
#include "Level_Edit.h"
#include "GameInstance.h"
#include "Map_Loader.h"

#include "imgui.h"

namespace
{
	void Draw_MapPreviewButtons(CLevel_Edit* pLevel, _int* pInOutPresetIndex)
	{
		if (nullptr == pLevel || nullptr == pInOutPresetIndex)
			return;

		const _uint iPresetCount = Client::CMap_Loader::Get_MapPresetCount();
		if (0 == iPresetCount)
		{
			ImGui::TextDisabled("Map preset unavailable.");
			return;
		}

		if (*pInOutPresetIndex < 0 || static_cast<_uint>(*pInOutPresetIndex) >= iPresetCount)
			*pInOutPresetIndex = 0;

		ImGui::SetNextItemWidth(120.f);
		if (ImGui::BeginCombo(
			"##MapPreviewPreset",
			Client::CMap_Loader::Get_MapPresetLabel(static_cast<_uint>(*pInOutPresetIndex))))
		{
			for (_uint i = 0; i < iPresetCount; ++i)
			{
				const _bool bSelected = (static_cast<_uint>(*pInOutPresetIndex) == i);
				if (ImGui::Selectable(CMap_Loader::Get_MapPresetLabel(i), bSelected))
					*pInOutPresetIndex = static_cast<_int>(i);

				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (ImGui::Button("Load All"))
			pLevel->Load_MapPreview(static_cast<_uint>(*pInOutPresetIndex));

		ImGui::SameLine();
		if (ImGui::Button("Load Stage"))
			pLevel->Load_MapPreviewStage(static_cast<_uint>(*pInOutPresetIndex));

		ImGui::SameLine();
		if (ImGui::Button("Load Env"))
			pLevel->Load_MapPreviewEnv(static_cast<_uint>(*pInOutPresetIndex));

		ImGui::SameLine();
		if (ImGui::Button("Clear All"))
			pLevel->Clear_MapPreview();

		ImGui::SameLine();
		if (ImGui::Button("Clear Stage"))
			pLevel->Clear_MapPreviewStage();

		ImGui::SameLine();
		if (ImGui::Button("Clear Env"))
			pLevel->Clear_MapPreviewEnv();

		ImGui::SameLine();
		if (ImGui::Button("Map Override Save"))
		{
			if (FAILED(pLevel->Save_MapOverride()))
				MSG_BOX("MAP OVERRIDE SAVE FAILED");
		}

		if (ImGui::IsItemHovered())
		{
			_wstring strOverridePath;
			if (SUCCEEDED(CMap_Loader::Get_MapPresetOverrideAssetPath(static_cast<_uint>(*pInOutPresetIndex), L"", &strOverridePath)))
			{
				const string strTooltip = WstrToStr(strOverridePath);
				ImGui::SetTooltip("%s", strTooltip.c_str());
			}
		}
	}

	void Draw_MapPreviewStatus(CLevel_Edit* pLevel)
	{
		if (nullptr == pLevel)
			return;

		const wstring& strStatus = pLevel->Get_MapPreviewStatus();
		if (strStatus.empty())
			return;

		string strDisplay(strStatus.begin(), strStatus.end());
		ImGui::TextDisabled("%s", strDisplay.c_str());
	}
}

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

	static _int s_iMapPreviewPreset = 0;

	Draw_EditButtons(pLevel);
	ImGui::SameLine();
	Draw_MapPreviewButtons(pLevel, &s_iMapPreviewPreset);

	Draw_GizmoButtons();
	ImGui::SameLine();

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

	Draw_MapPreviewStatus(pLevel);

	End_Panel();
}

void CPanel_Toolbar::Draw_EditButtons(CLevel_Edit* pLevel)
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();

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
			if (bActive)
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
			if (ImGui::Button(label))
				pEI->Set_GizmoOp(op);
			if (bActive)
				ImGui::PopStyleColor();
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