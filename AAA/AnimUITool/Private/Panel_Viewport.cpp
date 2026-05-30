#include "Panel_Viewport.h"
#include "imgui.h"

using namespace AnimUITool;

CPanel_Viewport::CPanel_Viewport(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Viewport");
}

void CPanel_Viewport::Render()
{
    ImGui::Begin(m_szName);

    ImVec2 vAvail = ImGui::GetContentRegionAvail();
    if (vAvail.x >= 1.f && vAvail.y >= 1.f)
    {
        const float fAvailAspect = vAvail.x / vAvail.y;
        ImVec2 vSize;
        if (fAvailAspect > m_fTargetAspect) { vSize.y = vAvail.y; vSize.x = vAvail.y * m_fTargetAspect; }
        else { vSize.x = vAvail.x; vSize.y = vAvail.x / m_fTargetAspect; }

        ImVec2 vOffset((vAvail.x - vSize.x) * 0.5f, (vAvail.y - vSize.y) * 0.5f);
        ImVec2 vCursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(vCursor.x + vOffset.x, vCursor.y + vOffset.y));

        ImVec2 vImagePos = ImGui::GetCursorScreenPos();

        if (m_pSRV)
            ImGui::Image((ImTextureID)m_pSRV, vSize);
        else
            ImGui::Dummy(vSize);

        m_bHovered = ImGui::IsItemHovered();

        if (ImGui::IsItemHovered())
        {
            ImVec2 m = ImGui::GetMousePos();
            float ndcX = ((m.x - vImagePos.x) / vSize.x) * 2.f - 1.f;
            float ndcY = 1.f - ((m.y - vImagePos.y) / vSize.y) * 2.f;
            ImGui::SetTooltip("NDC: %.2f, %.2f", ndcX, ndcY);
        }
        else
            m_bHovered = false;
    }

    ImGui::End();
}

void CPanel_Viewport::Set_SRV(ID3D11ShaderResourceView* pSRV)
{
    if (m_pSRV == pSRV)
        return;

    Safe_Release(m_pSRV);

    m_pSRV = pSRV;

    Safe_AddRef(m_pSRV);
}

CPanel_Viewport* CPanel_Viewport::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Viewport(pDevice, pContext);
}

void CPanel_Viewport::Free() 
{
    Safe_Release(m_pSRV);

    __super::Free();
}