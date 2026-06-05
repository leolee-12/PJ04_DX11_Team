#include "Panel_Hierarchy.h"
#include "Panel_Manager.h"
#include "Preview_Actor.h"
#include "Level_Tool.h"
#include "UIContainerObject.h"
#include "UIPartObject.h"

CPanel_Hierarchy::CPanel_Hierarchy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Hierarchy");
}

void CPanel_Hierarchy::Render()
{
    ImGui::Begin(m_szName);

    if (m_pPanel_Manager->Get_WorkMode() == TOOL_MODE::UI)
        Render_UIHierarchy();
    else
        Render_AnimationHierarchy();

    ImGui::End();
}

void CPanel_Hierarchy::Render_AnimationHierarchy()
{
    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();

    if (ctx.pActor)
    {
        std::string name(ctx.strName.begin(), ctx.strName.end());
        if (name.empty())
            name = "Preview";

        const float fAvail = ImGui::GetContentRegionAvail().x;
        _bool bSelected = (m_pPanel_Manager->Get_Selected() == ctx.pActor);

        if (ImGui::Selectable(name.c_str(), bSelected, 0, ImVec2(fAvail - 60.f, 0.f)))
            m_pPanel_Manager->Set_Selected(ctx.pActor);

        ImGui::SameLine();

        ImGui::PushID(ctx.pActor);
        _bool bDelete = ImGui::SmallButton("Delete");
        ImGui::PopID();

        if (bSelected && ImGui::IsKeyPressed(ImGuiKey_Delete))
            bDelete = true;

        if (bDelete)
        {
            m_pPanel_Manager->Clear_Preview();
            return;
        }

        ImGui::TextDisabled("Type: %s", ctx.pActor->Get_Type() == MODEL::ANIM ? "ANIM" : "NONANIM");
    }
    else
    {
        ImGui::TextDisabled("(no model loaded)");
    }
}

void CPanel_Hierarchy::Render_UIHierarchy()
{
    CLevel_Tool* pLevel = m_pPanel_Manager->Get_Level();
    if (nullptr == pLevel)
    {
        ImGui::TextDisabled("(no level)");
        return;
    }

    m_pPanel_Manager->Validate_UISelection();

    UI_CONTEXT& UIContext = m_pPanel_Manager->Get_UIContext();
    const auto& UIContainers = pLevel->Get_UIContainers();

    if (UIContext.bDirty)
        ImGui::TextColored(ImVec4(1.f, 0.75f, 0.25f, 1.f), "Modified");

    if (UIContainers.empty())
    {
        ImGui::TextDisabled("(no ui containers)");
        return;
    }

    for (auto* pContainer : UIContainers)
    {
        if (nullptr == pContainer)
            continue;

        std::string strContainerName = ToUtf8(pContainer->Get_ObjectTag());
        if (strContainerName.empty())
            strContainerName = "UIContainer";

        ImGui::PushID(pContainer);

        ImGuiTreeNodeFlags eNodeFlags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth;

        const _bool bOpened = ImGui::TreeNodeEx(strContainerName.c_str(), eNodeFlags);

        if (bOpened)
        {
            const auto& UIParts = pContainer->Get_UIPartObjects();

            if (UIParts.empty())
            {
                ImGui::TextDisabled("(no parts)");
            }
            else
            {
                for (const auto& Pair : UIParts)
                {
                    const _wstring& strPartTag = Pair.first;
                    CUIPartObject* pPart = Pair.second;

                    if (nullptr == pPart)
                        continue;

                    std::string strPartName = ToUtf8(strPartTag);
                    if (strPartName.empty())
                        strPartName = "UIPart";

                    const _bool bSelected =
                        UIContext.Selection.pContainer == pContainer &&
                        UIContext.Selection.pPart == pPart &&
                        UIContext.Selection.strPartTag == strPartTag;

                    ImGui::PushID(pPart);

                    if (ImGui::Selectable(strPartName.c_str(), bSelected, ImGuiSelectableFlags_SpanAvailWidth))
                        m_pPanel_Manager->Set_UISelected(pContainer, pPart, strPartTag);

                    if (bSelected)
                    {
                        ImGui::TextDisabled(
                            "Layer: %s / Z: %.3f",
                            Get_RenderLayerName(pPart->Get_RenderLayer()),
                            pPart->Get_ZOrder());
                    }

                    ImGui::PopID();
                }
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}

const _char* CPanel_Hierarchy::Get_RenderLayerName(RENDERUIID eRenderLayer)
{
    switch (eRenderLayer)
    {
    case RENDERUIID::BACK:
        return "BACK";

    case RENDERUIID::MIDDLE:
        return "MIDDLE";

    case RENDERUIID::FRONT:
        return "FRONT";

    default:
        return "UNKNOWN";
    }
}

CPanel_Hierarchy* CPanel_Hierarchy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Hierarchy(pDevice, pContext);
}

void CPanel_Hierarchy::Free()
{
    __super::Free();
}