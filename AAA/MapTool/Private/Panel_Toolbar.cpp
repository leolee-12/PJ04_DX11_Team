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

		const _uint iPresetCount = Client::CMap_Loader::Get_MapCount();
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
			Client::CMap_Loader::Get_MapName(static_cast<_uint>(*pInOutPresetIndex))))
		{
			for (_uint i = 0; i < iPresetCount; ++i)
			{
				const _bool bSelected = (static_cast<_uint>(*pInOutPresetIndex) == i);
				if (ImGui::Selectable(CMap_Loader::Get_MapName(i), bSelected))
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
		if (ImGui::Button("Load LD"))
			pLevel->Load_LDPreview(static_cast<_uint>(*pInOutPresetIndex));

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
		if (ImGui::Button("Clear LD"))
			pLevel->Clear_LDPreview();

		ImGui::SameLine();
		if (ImGui::Button("Place Edit Save"))
		{
			if (FAILED(pLevel->Save_PlaceEdit()))
				MSG_BOX("PLACE EDIT SAVE FAILED");
		}

		if (ImGui::IsItemHovered())
		{
			_wstring strEditFilePath;
			if (SUCCEEDED(CMap_Loader::Get_PresetEditFilePath(static_cast<_uint>(*pInOutPresetIndex), L"", &strEditFilePath)))
			{
				const string strTooltip = WstrToStr(strEditFilePath);
				ImGui::SetTooltip("%s", strTooltip.c_str());
			}
		}
	}

	void Draw_MapPreviewStatus(CLevel_Edit* pLevel)
	{
		if (nullptr == pLevel)
			return;

		const _wstring& wstrStatus = pLevel->Get_MapPreviewStatus();
		if (wstrStatus.empty())
			return;

		_string strDisplay = WstrToStr(wstrStatus);
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

	Draw_MapPreviewButtons(pLevel, &s_iMapPreviewPreset);

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

	{
		const bool bOn = m_pGI_Proxy->Is_PhysXDebug();

		if (bOn)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));

		if (ImGui::Button("Physics Debug"))
			m_pGI_Proxy->Toggle_PhysXDebug();

		if (bOn)
			ImGui::PopStyleColor();
	}

	ImGui::SameLine();
	Draw_CameraButtons(pLevel);

	Draw_MapPreviewStatus(pLevel);

	End_Panel();
}

void CPanel_Toolbar::Draw_CameraButtons(CLevel_Edit* pLevel)
{
	if (nullptr == pLevel)
		return;

	static _float s_fJumpStep = 25.f;
	static _float3 s_vTeleportPos = { 0.f, 5.f, -10.f };

	const _bool bEditMode = m_pGI_Proxy->Is_EditMode();

	if (ImGui::Button(bEditMode ? "Play" : "Back to Edit"))
	{
		m_pGI_Proxy->Set_EditMode(!bEditMode);

		if (!bEditMode)
			pLevel->Back_To_Edit();
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset Rot"))
		pLevel->Reset_EditCameraRotation();

	ImGui::SameLine();
	ImGui::Text("Step");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(80.f);
	ImGui::DragFloat("##CameraJumpStep", &s_fJumpStep, 5.f, 5.f, 500.f, "%.0f");

	ImGui::SameLine();
	if (ImGui::Button("F"))
		pLevel->Jump_EditCamera(s_fJumpStep, 0.f);

	ImGui::SameLine();
	if (ImGui::Button("B"))
		pLevel->Jump_EditCamera(-s_fJumpStep, 0.f);

	ImGui::SameLine();
	if (ImGui::Button("L"))
		pLevel->Jump_EditCamera(0.f, -s_fJumpStep);

	ImGui::SameLine();
	if (ImGui::Button("R"))
		pLevel->Jump_EditCamera(0.f, s_fJumpStep);

	ImGui::SameLine();
	ImGui::Text("Pos");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(180.f);
	ImGui::DragFloat3("##CameraTeleportPos", (float*)&s_vTeleportPos, 1.f, 0.f, 0.f, "%.1f");

	ImGui::SameLine();
	if (ImGui::Button("Teleport"))
		pLevel->Teleport_EditCamera(s_vTeleportPos);
}

CPanel_Toolbar* CPanel_Toolbar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CPanel_Toolbar(pDevice, pContext);
}

void CPanel_Toolbar::Free()
{
	__super::Free();
}
