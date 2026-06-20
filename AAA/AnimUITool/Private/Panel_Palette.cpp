#include "Panel_Palette.h"
#include "Panel_Manager.h"
#include "Level_Tool.h"
#include "GameObject_Factory.h"


CPanel_Palette::CPanel_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPanel{ pDevice, pContext }
{
	strcpy_s(m_szName, "Palette");
}

void CPanel_Palette::Render()
{
	ImGui::Begin(m_szName);

	CLevel_Tool* pLevel = m_pPanel_Manager->Get_Level();

	if (nullptr == pLevel)
	{
		ImGui::TextDisabled("No Level");
		ImGui::End();
		return;
	}

    map<_wstring, vector<_wstring>> TagsByCategory;
    Client::CGameObject_Factory::GetInstance()->Copy_TagsByCategory(&TagsByCategory);

    for (auto& [category, tags] : TagsByCategory)
    {
        if (category.rfind(L"UI_", 0) == 0)
            continue;

        string strCat = WstrToStr(category);
        if (ImGui::TreeNode(strCat.c_str()))
        {
            for (auto& strTag : tags)
            {
                string strLabel = WstrToStr(strTag);
                if (ImGui::Button(strLabel.c_str()))
                {
                    _wstring strName = strTag + L"_" + to_wstring(m_iSpawnCounter++);
                    pLevel->Spawn_Object(strTag, L"Layer_Object", strName);   
                }
            }
            ImGui::TreePop();
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Clear Spawned"))
        m_pPanel_Manager->Clear_Spawned();

    ImGui::End();
}

CPanel_Palette* CPanel_Palette::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) 
{
    return new CPanel_Palette(pDevice, pContext);
}

void CPanel_Palette::Free()
{
    __super::Free();
}

