#include "Panel_Hierarchy.h"

#include "EditInstance.h"
#include "Level_Edit.h"
#include "GameInstance.h"
#include "GameObject.h"

#include "imgui.h"

CPanel_Hierarchy::CPanel_Hierarchy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Hierarchy");
}

void CPanel_Hierarchy::Render()
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

    for (auto& [LayerTag, Objects] : pLevel->Get_Layers())
    {
        string strLayerName = WstrToStr(LayerTag);

        if (ImGui::TreeNode(strLayerName.c_str()))
        {
            // 레이어 노드를 드롭 타겟으로 (오브젝트를 다른 레이어로 이동)
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECT_DND"))
                {
                    CGameObject* pDropped = *(CGameObject**)payload->Data;
                    pLevel->Change_ObjectLayer(pDropped, LayerTag);
                }
                ImGui::EndDragDropTarget();
            }

            for (auto& Handle : Objects)
            {
                string strName = WstrToStr(Handle.strName);
                _bool bSelected = (Handle.pObject == pLevel->Get_Selected());

                float fAvailWidth = ImGui::GetContentRegionAvail().x;

                if (ImGui::Selectable(strName.c_str(), bSelected, 0, ImVec2(fAvailWidth - 25.f, 0)))
                    pLevel->Set_Selected(Handle.pObject);

                // 드래그 소스
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    ImGui::SetDragDropPayload("OBJECT_DND", &Handle.pObject, sizeof(CGameObject*));
                    ImGui::Text("%s", strName.c_str());
                    ImGui::EndDragDropSource();
                }

                ImGui::SameLine();

                ImGui::PushID(Handle.pObject);
                if (ImGui::SmallButton("X"))
                {
                    pLevel->Delete_Object(Handle.pObject);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }

    End_Panel();
}

CPanel_Hierarchy* CPanel_Hierarchy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Hierarchy(pDevice, pContext);
}

void CPanel_Hierarchy::Free()
{
    __super::Free();
}
