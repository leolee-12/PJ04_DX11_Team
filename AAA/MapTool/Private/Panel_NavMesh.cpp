#include "Panel_NavMesh.h"
#include "imgui.h"

CPanel_NavMesh::CPanel_NavMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "NavMesh");
}

void CPanel_NavMesh::Render()
{
    if (!Begin_Panel())
    {
        End_Panel();
        return;
    }

    ImGui::TextColored(ImVec4(1.f, 0.7f, 0.2f, 1.f), "NavMesh editing : ON HOLD");
    ImGui::Separator();
    ImGui::TextWrapped(
        "Navigation pipeline is likely unused for this project, so the NavMesh "
        "editor is introduced as a slot only.");
    ImGui::Spacing();
    ImGui::TextDisabled("Full implementation reference: docs/Panel_MapTool.*");
    ImGui::Spacing();

    ImGui::BeginDisabled();
    ImGui::Checkbox("Enable Nav Edit Mode", &m_bEnabled);
    ImGui::EndDisabled();

    End_Panel();
}

CPanel_NavMesh* CPanel_NavMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_NavMesh(pDevice, pContext);
}

void CPanel_NavMesh::Free()
{
    __super::Free();
}
