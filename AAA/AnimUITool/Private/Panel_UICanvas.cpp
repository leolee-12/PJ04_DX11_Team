#include "Panel_UICanvas.h"
#include "imgui.h"

using namespace AnimUITool;

CPanel_UICanvas::CPanel_UICanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "UICanvas");
}

void CPanel_UICanvas::Render()
{
    ImGui::Begin(m_szName);

    ImGui::Text("Design: %.0f x %.0f", m_vDesignSize.x, m_vDesignSize.y);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &m_bShowGrid);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.f);
    ImGui::DragFloat("Step", &m_fGridStep, 1.f, 8.f, 400.f, "%.0f");

    ImGui::Separator();

    m_bHovered = false;
    m_bHasMouseCanvasPos = false;

    ImVec2 vAvail = ImGui::GetContentRegionAvail();
    if (vAvail.x >= 1.f && vAvail.y >= 1.f)
    {
        const _float fAvailAspect = vAvail.x / vAvail.y;

        ImVec2 vSize{};
        if (fAvailAspect > m_fTargetAspect)
        {
            vSize.y = vAvail.y;
            vSize.x = vAvail.y * m_fTargetAspect;
        }
        else
        {
            vSize.x = vAvail.x;
            vSize.y = vAvail.x / m_fTargetAspect;
        }

        ImVec2 vOffset(
            (vAvail.x - vSize.x) * 0.5f,
            (vAvail.y - vSize.y) * 0.5f);

        ImVec2 vCursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(vCursor.x + vOffset.x, vCursor.y + vOffset.y));

        ImVec2 vImagePos = ImGui::GetCursorScreenPos();

        m_vCanvasMin = { vImagePos.x, vImagePos.y };
        m_vCanvasSize = { vSize.x, vSize.y };

        if (m_pSRV)
            ImGui::Image((ImTextureID)m_pSRV, vSize);
        else
            ImGui::Dummy(vSize);

        m_bHovered = ImGui::IsItemHovered();

        if (m_bHovered)
        {
            ImVec2 vMouse = ImGui::GetMousePos();
            _float2 vScreenPos = { vMouse.x, vMouse.y };

            m_bHasMouseCanvasPos =
                ScreenToDesignPos(vScreenPos, &m_vMouseDesignPos) &&
                ScreenToUIPos(vScreenPos, &m_vMouseUIPos);
        }

        Draw_Grid(vImagePos.x, vImagePos.y, vSize.x, vSize.y);

        if (m_bHasMouseCanvasPos)
        {
            char szDebug[160]{};
            sprintf_s(
                szDebug,
                "Design: %.1f, %.1f | UI: %.1f, %.1f",
                m_vMouseDesignPos.x,
                m_vMouseDesignPos.y,
                m_vMouseUIPos.x,
                m_vMouseUIPos.y);

            ImGui::GetWindowDrawList()->AddText(
                ImVec2(vImagePos.x + 8.f, vImagePos.y + 8.f),
                IM_COL32(255, 255, 255, 255),
                szDebug);
        }
    }
    else
    {
        m_vCanvasMin = {};
        m_vCanvasSize = {};
    }

    ImGui::End();
}

void CPanel_UICanvas::Draw_Grid(_float fX, _float fY, _float fW, _float fH)
{
    if (!m_bShowGrid)
        return;

    if (m_fGridStep <= 1.f)
        return;

    const _float fScaleX = fW / m_vDesignSize.x;
    const _float fScaleY = fH / m_vDesignSize.y;
    const _float fStepX = m_fGridStep * fScaleX;
    const _float fStepY = m_fGridStep * fScaleY;

    if (fStepX < 4.f || fStepY < 4.f)
        return;

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    const ImVec2 vMin(fX, fY);
    const ImVec2 vMax(fX + fW, fY + fH);

    const _float fCenterX = fX + fW * 0.5f;
    const _float fCenterY = fY + fH * 0.5f;

    const ImU32 iGridColor = IM_COL32(255, 255, 255, 35);
    const ImU32 iAxisColor = IM_COL32(255, 255, 255, 95);
    const ImU32 iBorderColor = IM_COL32(255, 255, 255, 140);

    for (_float x = fCenterX; x <= vMax.x; x += fStepX)
        pDrawList->AddLine(ImVec2(x, vMin.y), ImVec2(x, vMax.y), iGridColor);

    for (_float x = fCenterX - fStepX; x >= vMin.x; x -= fStepX)
        pDrawList->AddLine(ImVec2(x, vMin.y), ImVec2(x, vMax.y), iGridColor);

    for (_float y = fCenterY; y <= vMax.y; y += fStepY)
        pDrawList->AddLine(ImVec2(vMin.x, y), ImVec2(vMax.x, y), iGridColor);

    for (_float y = fCenterY - fStepY; y >= vMin.y; y -= fStepY)
        pDrawList->AddLine(ImVec2(vMin.x, y), ImVec2(vMax.x, y), iGridColor);

    pDrawList->AddLine(ImVec2(fCenterX, vMin.y), ImVec2(fCenterX, vMax.y), iAxisColor);
    pDrawList->AddLine(ImVec2(vMin.x, fCenterY), ImVec2(vMax.x, fCenterY), iAxisColor);
    pDrawList->AddRect(vMin, vMax, iBorderColor);
}

void CPanel_UICanvas::Set_SRV(ID3D11ShaderResourceView* pSRV)
{
    if (m_pSRV == pSRV)
        return;

    Safe_Release(m_pSRV);

    m_pSRV = pSRV;

    Safe_AddRef(m_pSRV);
}

void CPanel_UICanvas::Set_DesignSize(_float fWidth, _float fHeight)
{
    if (fWidth <= 1.f || fHeight <= 1.f)
        return;

    m_vDesignSize = { fWidth, fHeight };
    m_fTargetAspect = fWidth / fHeight;
}

_bool CPanel_UICanvas::ScreenToDesignPos(const _float2& vScreenPos, _float2* pOutDesignPos) const
{
    if (!pOutDesignPos)
        return false;

    if (m_vCanvasSize.x <= 1.f || m_vCanvasSize.y <= 1.f)
        return false;

    const _float fLocalX = vScreenPos.x - m_vCanvasMin.x;
    const _float fLocalY = vScreenPos.y - m_vCanvasMin.y;

    if (fLocalX < 0.f || fLocalX > m_vCanvasSize.x ||
        fLocalY < 0.f || fLocalY > m_vCanvasSize.y)
        return false;

    const _float fU = fLocalX / m_vCanvasSize.x;
    const _float fV = fLocalY / m_vCanvasSize.y;

    pOutDesignPos->x = fU * m_vDesignSize.x;
    pOutDesignPos->y = fV * m_vDesignSize.y;

    return true;
}

_bool CPanel_UICanvas::ScreenToUIPos(const _float2& vScreenPos, _float2* pOutUIPos) const
{
    if (!pOutUIPos)
        return false;

    _float2 vDesignPos{};
    if (!ScreenToDesignPos(vScreenPos, &vDesignPos))
        return false;

    pOutUIPos->x = vDesignPos.x - m_vDesignSize.x * 0.5f;
    pOutUIPos->y = m_vDesignSize.y * 0.5f - vDesignPos.y;

    return true;
}

CPanel_UICanvas* CPanel_UICanvas::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_UICanvas(pDevice, pContext);
}

void CPanel_UICanvas::Free()
{
    Safe_Release(m_pSRV);

    __super::Free();
}