#include "Panel_Palette.h"

#include "EditInstance.h"
#include "Level_Edit.h"
#include "GameObject_Factory.h"
#include "LevelDesign_Registry.h"
#include "LD_DeformObject.h"

#include "GameInstance.h"

#include "imgui.h"

CPanel_Palette::CPanel_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPanel(pDevice, pContext)
{
	strcpy_s(m_szName, "Palette");
}

void CPanel_Palette::Render()
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

	map<wstring, vector<wstring>> TagsByCategory;
	CGameObject_Factory::GetInstance()->Copy_TagsByCategory(&TagsByCategory);

	for (auto& [category, tags] : TagsByCategory)
	{
		string strCatLabel = WstrToStr(category);
		if (ImGui::TreeNode(strCatLabel.c_str()))
		{
			for (auto& strTag : tags)
			{
				const LD_SPAWN_SPEC* pPlacementSpec = nullptr;

				if (category == L"LEVELDESIGN_OBJECT")
				{
					pPlacementSpec = CLevelDesign_Registry::Find_PlacementSpec(strTag);
					if (nullptr == pPlacementSpec)
						continue;
				}

				string strLabel = WstrToStr(strTag);
				if (ImGui::Button(strLabel.c_str()))
				{
					if (category == L"UI_OBJECT" || category == L"UI_CONTAINER_OBJECT")
					{
						m_strPendingTag = strTag;
						memset(m_szNameBuf, 0, sizeof(m_szNameBuf));
						m_bOpenNamePopup = true;
					}
					else if (category == L"ENV_TRIGGER")
					{
						pLevel->Begin_PlaceMode(strTag, L"Layer_EnvEffect");
					}
					else if (nullptr != pPlacementSpec)
					{
						pLevel->Begin_PlaceMode(strTag, pPlacementSpec->strLayerTag);
					}
					else if (category == L"DEFORM_OBJECT")
					{
						pLevel->Begin_PlaceMode(strTag, CLD_DeformObject::LAYER_TAG);
					}

				}
			}
			ImGui::TreePop();
		}
	}

	if (pLevel->Is_PlaceMode())
	{
		string strProto = WstrToStr(pLevel->Get_PendingProto());
		ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Placing : %s", strProto.c_str());
		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "[Esc : Cancel]");
	}

	if (m_bOpenNamePopup)
	{
		ImGui::OpenPopup("Input Name");
		m_bOpenNamePopup = false;
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Input Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Name:");
		ImGui::InputText("##name", m_szNameBuf, BUF_SIZE);

		if (ImGui::Button("OK"))
		{
			wstring strName = StrToWstr(m_szNameBuf);
			pLevel->Spawn_Object(m_strPendingTag, L"Default_Layer", strName);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	End_Panel();
}

CPanel_Palette* CPanel_Palette::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CPanel_Palette(pDevice, pContext);
}

void CPanel_Palette::Free()
{
	__super::Free();
}
