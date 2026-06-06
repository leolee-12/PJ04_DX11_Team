#include "Panel_UICanvas.h"
#include "Panel_Manager.h"
#include "Level_Tool.h"

#include "UIContainerObject.h"
#include "UIPartObject.h"
#include "Transform.h"

using namespace AnimUITool;

CPanel_UICanvas::CPanel_UICanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "UICanvas");
}

void CPanel_UICanvas::Render()
{
    ImGui::Begin(m_szName);

    UI_CONTEXT& UIContext = m_pPanel_Manager->Get_UIContext();

    if (UIContext.vDesignSize.x <= 1.f || UIContext.vDesignSize.y <= 1.f)
        UIContext.vDesignSize = { 1600.f, 900.f };

    m_fTargetAspect = UIContext.vDesignSize.x / UIContext.vDesignSize.y;

    ImGui::Text("Design: %.0f x %.0f", UIContext.vDesignSize.x, UIContext.vDesignSize.y);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &UIContext.bShowGrid);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.f);
    ImGui::DragFloat("Step", &UIContext.fGridStep, 1.f, 8.f, 400.f, "%.0f");

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
        Handle_Selection();
        Draw_SelectedPart();

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
    const UI_CONTEXT& UIContext = m_pPanel_Manager->Get_UIContext();

    if (!UIContext.bShowGrid)
        return;

    if (UIContext.fGridStep <= 1.f)
        return;

    const _float fScaleX = fW / UIContext.vDesignSize.x;
    const _float fScaleY = fH / UIContext.vDesignSize.y;
    const _float fStepX = UIContext.fGridStep * fScaleX;
    const _float fStepY = UIContext.fGridStep * fScaleY;

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

void CPanel_UICanvas::Handle_Selection()
{
    if (m_eDragMode != UI_DRAG_MODE::NONE)
    {
        if (m_bHasMouseCanvasPos && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            Update_Drag(m_vMouseUIPos);
        else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            End_Drag();

        return;
    }

    if (!m_bHasMouseCanvasPos)
        return;

    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;

    ImVec2 vMouse = ImGui::GetMousePos();
    _float2 vMouseScreen = { vMouse.x, vMouse.y };

    UI_DRAG_MODE eHandleMode = UI_DRAG_MODE::NONE;
    if (Hit_SelectedHandle(vMouseScreen, &eHandleMode))
    {
        Begin_Drag(eHandleMode, m_vMouseUIPos);
        return;
    }

    UI_PICK_CANDIDATE Candidate{};
    if (Pick_TopmostPart(m_vMouseUIPos, &Candidate))
    {
        m_pPanel_Manager->Set_UISelected(
            Candidate.pContainer,
            Candidate.pPart,
            Candidate.strPartTag);

        Begin_Drag(UI_DRAG_MODE::MOVE, m_vMouseUIPos);
        return;
    }

    Clear_UISelection();
}

void CPanel_UICanvas::Draw_SelectedPart()
{
    if (!m_pPanel_Manager->Validate_UISelection())
        return;

    const UI_SELECTION& Selection = m_pPanel_Manager->Get_UIContext().Selection;

    UI_PART_BOUNDS Bounds{};
    if (!Build_PartBounds(Selection.pContainer, Selection.pPart, &Bounds))
        return;

    const _float fHalfW = Bounds.vSize.x * 0.5f;
    const _float fHalfH = Bounds.vSize.y * 0.5f;

    _float2 vTL = { Bounds.vCenter.x - fHalfW, Bounds.vCenter.y + fHalfH };
    _float2 vTR = { Bounds.vCenter.x + fHalfW, Bounds.vCenter.y + fHalfH };
    _float2 vBL = { Bounds.vCenter.x - fHalfW, Bounds.vCenter.y - fHalfH };
    _float2 vBR = { Bounds.vCenter.x + fHalfW, Bounds.vCenter.y - fHalfH };

    _float2 sTL{}, sTR{}, sBL{}, sBR{};
    if (!UIToScreenPos(vTL, &sTL) || !UIToScreenPos(vTR, &sTR) ||
        !UIToScreenPos(vBL, &sBL) || !UIToScreenPos(vBR, &sBR))
        return;

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    pDrawList->AddRect(
        ImVec2(sTL.x, sTL.y),
        ImVec2(sBR.x, sBR.y),
        IM_COL32(255, 210, 80, 255),
        0.f,
        0,
        2.f);

    const _float fHandleHalf = 5.f;
    auto DrawHandle = [&](const _float2& vScreen)
        {
            pDrawList->AddRectFilled(
                ImVec2(vScreen.x - fHandleHalf, vScreen.y - fHandleHalf),
                ImVec2(vScreen.x + fHandleHalf, vScreen.y + fHandleHalf),
                IM_COL32(255, 210, 80, 255));

            pDrawList->AddRect(
                ImVec2(vScreen.x - fHandleHalf, vScreen.y - fHandleHalf),
                ImVec2(vScreen.x + fHandleHalf, vScreen.y + fHandleHalf),
                IM_COL32(30, 30, 30, 255));
        };

    DrawHandle(sTL);
    DrawHandle(sTR);
    DrawHandle(sBL);
    DrawHandle(sBR);
}

void CPanel_UICanvas::Clear_UISelection()
{
    m_eDragMode = UI_DRAG_MODE::NONE;
    m_pPanel_Manager->Clear_UISelected();
}

_bool CPanel_UICanvas::Build_PartBounds(CUIContainerObject* pContainer, CUIPartObject* pPart, UI_PART_BOUNDS* pOutBounds) const
{
    if (!pContainer || !pPart || !pOutBounds)
        return false;

    CTransform* pContainerTransform = pContainer->Get_Transform();
    CTransform* pPartTransform = pPart->Get_Transform();

    if (!pContainerTransform || !pPartTransform)
        return false;

    _matrix PartMatrix = XMLoadFloat4x4(pPartTransform->Get_WorldMatrixPtr());
    _matrix ContainerMatrix = XMLoadFloat4x4(pContainerTransform->Get_WorldMatrixPtr());

    _float4x4 Combined{};
    XMStoreFloat4x4(&Combined, PartMatrix * ContainerMatrix);

    const _float fScaleX = XMVectorGetX(XMVector3Length(XMVectorSet(Combined._11, Combined._12, Combined._13, 0.f)));
    const _float fScaleY = XMVectorGetX(XMVector3Length(XMVectorSet(Combined._21, Combined._22, Combined._23, 0.f)));

    if (fScaleX <= 0.f || fScaleY <= 0.f)
        return false;

    pOutBounds->vCenter = { Combined._41, Combined._42 };
    pOutBounds->vSize = { fScaleX, fScaleY };

    // Renderer 정렬 기준과 맞춘다. 현재 Get_ZOrder는 Part transform의 z를 반환한다.
    pOutBounds->fZ = pPart->Get_ZOrder();
    pOutBounds->eRenderLayer = pPart->Get_RenderLayer();

    return true;
}

_bool CPanel_UICanvas::Pick_TopmostPart(const _float2& vMouseUI, UI_PICK_CANDIDATE* pOutCandidate) const
{
    if (!pOutCandidate)
        return false;

    CLevel_Tool* pLevel = m_pPanel_Manager->Get_Level();
    if (!pLevel)
        return false;

    _bool bFound = false;
    UI_PICK_CANDIDATE Best{};

    for (auto* pContainer : pLevel->Get_UIContainers())
    {
        if (!pContainer)
            continue;

        const auto& Parts = pContainer->Get_UIPartObjects();
        for (const auto& Pair : Parts)
        {
            CUIPartObject* pPart = Pair.second;
            if (!pPart)
                continue;

            UI_PART_BOUNDS Bounds{};
            if (!Build_PartBounds(pContainer, pPart, &Bounds))
                continue;

            const _float fHalfW = Bounds.vSize.x * 0.5f;
            const _float fHalfH = Bounds.vSize.y * 0.5f;

            if (vMouseUI.x < Bounds.vCenter.x - fHalfW ||
                vMouseUI.x > Bounds.vCenter.x + fHalfW ||
                vMouseUI.y < Bounds.vCenter.y - fHalfH ||
                vMouseUI.y > Bounds.vCenter.y + fHalfH)
                continue;

            const _int iNewLayer = static_cast<_int>(Bounds.eRenderLayer);
            const _int iBestLayer = static_cast<_int>(Best.Bounds.eRenderLayer);

            constexpr _float fZEqualEpsilon = 0.0001f;

            _bool bTake = false;
            if (!bFound)
            {
                bTake = true;
            }
            else if (iNewLayer > iBestLayer)
            {
                bTake = true;
            }
            else if (iNewLayer == iBestLayer)
            {
                if (Bounds.fZ < Best.Bounds.fZ - fZEqualEpsilon)
                {
                    bTake = true;
                }
                else if (Bounds.fZ >= Best.Bounds.fZ - fZEqualEpsilon &&
                    Bounds.fZ <= Best.Bounds.fZ + fZEqualEpsilon)
                {
                    // 동일한 Render Layer, 같은 Z 값을 가진 오브젝트는 순회에서 나중에 발견한 후보가 선택됨
                    // 이렇게 되면 보편적으로 더 위에 배치되어 보이는 녀석이 선택됨
                    bTake = true;
                }
            }

            if (bTake)
            {
                bFound = true;
                Best.pContainer = pContainer;
                Best.pPart = pPart;
                Best.strPartTag = Pair.first;
                Best.Bounds = Bounds;
            }
        }
    }

    if (!bFound)
        return false;

    *pOutCandidate = Best;
    return true;
}

_bool CPanel_UICanvas::Hit_SelectedHandle(const _float2& vMouseScreen, UI_DRAG_MODE* pOutMode) const
{
    if (!pOutMode)
        return false;

    *pOutMode = UI_DRAG_MODE::NONE;

    if (!m_pPanel_Manager->Validate_UISelection())
        return false;

    const UI_SELECTION& Selection = m_pPanel_Manager->Get_UIContext().Selection;

    UI_PART_BOUNDS Bounds{};
    if (!Build_PartBounds(Selection.pContainer, Selection.pPart, &Bounds))
        return false;

    const _float fHalfW = Bounds.vSize.x * 0.5f;
    const _float fHalfH = Bounds.vSize.y * 0.5f;

    _float2 vTL = { Bounds.vCenter.x - fHalfW, Bounds.vCenter.y + fHalfH };
    _float2 vTR = { Bounds.vCenter.x + fHalfW, Bounds.vCenter.y + fHalfH };
    _float2 vBL = { Bounds.vCenter.x - fHalfW, Bounds.vCenter.y - fHalfH };
    _float2 vBR = { Bounds.vCenter.x + fHalfW, Bounds.vCenter.y - fHalfH };

    _float2 sTL{}, sTR{}, sBL{}, sBR{};
    if (!UIToScreenPos(vTL, &sTL) || !UIToScreenPos(vTR, &sTR) ||
        !UIToScreenPos(vBL, &sBL) || !UIToScreenPos(vBR, &sBR))
        return false;

    const _float fHandleHalf = 7.f;

    auto Hit = [&](const _float2& vHandleScreen)
        {
            const _float fDx = vMouseScreen.x - vHandleScreen.x;
            const _float fDy = vMouseScreen.y - vHandleScreen.y;

            return fDx >= -fHandleHalf && fDx <= fHandleHalf &&
                fDy >= -fHandleHalf && fDy <= fHandleHalf;
        };

    if (Hit(sTL)) { *pOutMode = UI_DRAG_MODE::RESIZE_TL; return true; }
    if (Hit(sTR)) { *pOutMode = UI_DRAG_MODE::RESIZE_TR; return true; }
    if (Hit(sBL)) { *pOutMode = UI_DRAG_MODE::RESIZE_BL; return true; }
    if (Hit(sBR)) { *pOutMode = UI_DRAG_MODE::RESIZE_BR; return true; }

    return false;
}

void CPanel_UICanvas::Begin_Drag(UI_DRAG_MODE eMode, const _float2& vMouseUI)
{
    if (eMode == UI_DRAG_MODE::NONE)
        return;

    if (!m_pPanel_Manager->Validate_UISelection())
        return;

    const UI_SELECTION& Selection = m_pPanel_Manager->Get_UIContext().Selection;

    CTransform* pTransform = Selection.pPart->Get_Transform();
    if (!pTransform)
        return;

    _vector vPos = pTransform->Get_State(STATE::POSITION);
    _float3 vScale = pTransform->Get_Scaled();

    m_eDragMode = eMode;
    m_vDragStartMouseUI = vMouseUI;
    m_vDragStartCenter = { XMVectorGetX(vPos), XMVectorGetY(vPos) };
    m_vDragStartSize = { vScale.x, vScale.y };
    m_fDragStartZ = XMVectorGetZ(vPos);
}

void CPanel_UICanvas::Update_Drag(const _float2& vMouseUI)
{
    if (m_eDragMode == UI_DRAG_MODE::NONE)
        return;

    if (!m_pPanel_Manager->Validate_UISelection())
    {
        End_Drag();
        return;
    }

    UI_SELECTION& Selection = m_pPanel_Manager->Get_UIContext().Selection;

    CTransform* pTransform = Selection.pPart->Get_Transform();
    if (!pTransform)
    {
        End_Drag();
        return;
    }

    _float2 vUIDelta =
    {
        vMouseUI.x - m_vDragStartMouseUI.x,
        vMouseUI.y - m_vDragStartMouseUI.y
    };

    _float2 vLocalDelta{};
    if (!UIToLocalDelta(vUIDelta, &vLocalDelta))
        vLocalDelta = vUIDelta;

    _float2 vNewCenter = m_vDragStartCenter;
    _float2 vNewSize = m_vDragStartSize;

    switch (m_eDragMode)
    {
    case UI_DRAG_MODE::MOVE:
        vNewCenter.x = m_vDragStartCenter.x + vLocalDelta.x;
        vNewCenter.y = m_vDragStartCenter.y + vLocalDelta.y;
        break;

    case UI_DRAG_MODE::RESIZE_TL:
        vNewSize.x = m_vDragStartSize.x - vLocalDelta.x;
        vNewSize.y = m_vDragStartSize.y + vLocalDelta.y;
        vNewCenter.x = m_vDragStartCenter.x + vLocalDelta.x * 0.5f;
        vNewCenter.y = m_vDragStartCenter.y + vLocalDelta.y * 0.5f;
        break;

    case UI_DRAG_MODE::RESIZE_TR:
        vNewSize.x = m_vDragStartSize.x + vLocalDelta.x;
        vNewSize.y = m_vDragStartSize.y + vLocalDelta.y;
        vNewCenter.x = m_vDragStartCenter.x + vLocalDelta.x * 0.5f;
        vNewCenter.y = m_vDragStartCenter.y + vLocalDelta.y * 0.5f;
        break;

    case UI_DRAG_MODE::RESIZE_BL:
        vNewSize.x = m_vDragStartSize.x - vLocalDelta.x;
        vNewSize.y = m_vDragStartSize.y - vLocalDelta.y;
        vNewCenter.x = m_vDragStartCenter.x + vLocalDelta.x * 0.5f;
        vNewCenter.y = m_vDragStartCenter.y + vLocalDelta.y * 0.5f;
        break;

    case UI_DRAG_MODE::RESIZE_BR:
        vNewSize.x = m_vDragStartSize.x + vLocalDelta.x;
        vNewSize.y = m_vDragStartSize.y - vLocalDelta.y;
        vNewCenter.x = m_vDragStartCenter.x + vLocalDelta.x * 0.5f;
        vNewCenter.y = m_vDragStartCenter.y + vLocalDelta.y * 0.5f;
        break;
    }

    const _float fMinSize = 8.f;
    if (vNewSize.x < fMinSize) vNewSize.x = fMinSize;
    if (vNewSize.y < fMinSize) vNewSize.y = fMinSize;

    pTransform->Set_Scale(vNewSize.x, vNewSize.y, 1.f);
    pTransform->Set_State(
        STATE::POSITION,
        XMVectorSet(vNewCenter.x, vNewCenter.y, m_fDragStartZ, 1.f));

    m_pPanel_Manager->Get_UIContext().bDirty = true;
}

void CPanel_UICanvas::End_Drag()
{
    m_eDragMode = UI_DRAG_MODE::NONE;
}

_bool CPanel_UICanvas::UIToScreenPos(const _float2& vUIPos, _float2* pOutScreenPos) const
{
    if (!pOutScreenPos)
        return false;

    if (m_vCanvasSize.x <= 1.f || m_vCanvasSize.y <= 1.f)
        return false;

    const UI_CONTEXT& UIContext = m_pPanel_Manager->Get_UIContext();

    const _float fDesignX = vUIPos.x + UIContext.vDesignSize.x * 0.5f;
    const _float fDesignY = UIContext.vDesignSize.y * 0.5f - vUIPos.y;

    pOutScreenPos->x = m_vCanvasMin.x + (fDesignX / UIContext.vDesignSize.x) * m_vCanvasSize.x;
    pOutScreenPos->y = m_vCanvasMin.y + (fDesignY / UIContext.vDesignSize.y) * m_vCanvasSize.y;

    return true;
}

_bool CPanel_UICanvas::UIToLocalDelta(const _float2& vUIDelta, _float2* pOutLocalDelta) const
{
    if (!pOutLocalDelta)
        return false;

    if (!m_pPanel_Manager->Validate_UISelection())
        return false;

    const UI_SELECTION& Selection = m_pPanel_Manager->Get_UIContext().Selection;

    CTransform* pContainerTransform = Selection.pContainer->Get_Transform();
    if (!pContainerTransform)
        return false;

    _matrix ParentMatrix = XMLoadFloat4x4(pContainerTransform->Get_WorldMatrixPtr());
    _matrix InvParentMatrix = XMMatrixInverse(nullptr, ParentMatrix);

    _vector vWorldDelta = XMVectorSet(vUIDelta.x, vUIDelta.y, 0.f, 0.f);
    _vector vLocalDelta = XMVector3TransformNormal(vWorldDelta, InvParentMatrix);

    pOutLocalDelta->x = XMVectorGetX(vLocalDelta);
    pOutLocalDelta->y = XMVectorGetY(vLocalDelta);

    return true;
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

    UI_CONTEXT& UIContext = m_pPanel_Manager->Get_UIContext();

    UIContext.vDesignSize = { fWidth, fHeight };
    m_fTargetAspect = fWidth / fHeight;
}

_bool CPanel_UICanvas::ScreenToDesignPos(const _float2& vScreenPos, _float2* pOutDesignPos) const
{
    if (!pOutDesignPos)
        return false;

    if (m_vCanvasSize.x <= 1.f || m_vCanvasSize.y <= 1.f)
        return false;

    const UI_CONTEXT& UIContext = m_pPanel_Manager->Get_UIContext();

    const _float fLocalX = vScreenPos.x - m_vCanvasMin.x;
    const _float fLocalY = vScreenPos.y - m_vCanvasMin.y;

    if (fLocalX < 0.f || fLocalX > m_vCanvasSize.x ||
        fLocalY < 0.f || fLocalY > m_vCanvasSize.y)
        return false;

    const _float fU = fLocalX / m_vCanvasSize.x;
    const _float fV = fLocalY / m_vCanvasSize.y;

    pOutDesignPos->x = fU * UIContext.vDesignSize.x;
    pOutDesignPos->y = fV * UIContext.vDesignSize.y;

    return true;
}

_bool CPanel_UICanvas::ScreenToUIPos(const _float2& vScreenPos, _float2* pOutUIPos) const
{
    if (!pOutUIPos)
        return false;

    const UI_CONTEXT& UIContext = m_pPanel_Manager->Get_UIContext();

    _float2 vDesignPos{};
    if (!ScreenToDesignPos(vScreenPos, &vDesignPos))
        return false;

    pOutUIPos->x = vDesignPos.x - UIContext.vDesignSize.x * 0.5f;
    pOutUIPos->y = UIContext.vDesignSize.y * 0.5f - vDesignPos.y;

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