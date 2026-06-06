#include "Panel_Hierarchy.h"
#include "Panel_Manager.h"
#include "Preview_Actor.h"
#include "imgui.h"

CPanel_Hierarchy::CPanel_Hierarchy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Hierarchy");
}

void CPanel_Hierarchy::Render()
{
    ImGui::Begin(m_szName);

    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
    if (ctx.pActor)
    {
        std::string name(ctx.strName.begin(), ctx.strName.end());
        if (name.empty()) name = "Preview";

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
            ImGui::End();
            return;
        }

        ImGui::TextDisabled("Type: %s", ctx.pActor->Get_Type() == MODEL::ANIM ? "ANIM" : "NONANIM");
    }
    else
        ImGui::TextDisabled("(no model loaded)");

    ImGui::End();
}

CPanel_Hierarchy* CPanel_Hierarchy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Hierarchy(pDevice, pContext);
}

void CPanel_Hierarchy::Free()
{
    __super::Free();
}