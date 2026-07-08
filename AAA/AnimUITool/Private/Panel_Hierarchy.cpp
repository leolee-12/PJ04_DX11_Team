#include "Panel_Hierarchy.h"
#include "Panel_Manager.h"
#include "Preview_Actor.h"
#include "Level_Tool.h"
#include "UIContainerObject.h"
#include "UIPartObject.h"
#include "UI_Image.h"
#include "UI_SpriteAnim.h"
#include "UI_GaugeFill.h"
#include "UI_Curtain.h"
#include "UI_CurtainAnimBase.h"
#include "ContainerObject.h"
#include "PartObject.h"
#include "Model.h"
#include "Animator.h"
#include "UI_CoordinatorContainer.h"

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
    CLevel_Tool* pLevel = m_pPanel_Manager->Get_Level();
    if (!pLevel) { ImGui::TextDisabled("(no level)"); return; }

    m_pPanel_Manager->Validate_AnimSelection();

    // draw one row, return true if Delete requested
    auto DrawRow = [&](CGameObject* pObj, const char* szFallback) -> bool
        {
            ImGui::PushID(pObj);
            std::string name = ToUtf8(pObj->Get_ObjectTag());
            if (name.empty()) name = szFallback;

            const _bool bSel = (m_pPanel_Manager->Get_Selected() == pObj);
            const float fAvail = ImGui::GetContentRegionAvail().x;

            if (ImGui::Selectable(name.c_str(), bSel, 0, ImVec2(fAvail - 60.f, 0.f)))
                m_pPanel_Manager->Bind_ForAnim(pObj);          // select -> bind into ctx

            ImGui::SameLine();
            _bool bDel = ImGui::SmallButton("Delete");
            if (bSel && ImGui::IsKeyPressed(ImGuiKey_Delete)) bDel = true;
            ImGui::PopID();
            return bDel;
        };

    // 1) Preview (raw extracted-model verification)
    ImGui::SeparatorText("Preview");
    if (CGameObject* pPreview = pLevel->Get_Preview())
    {
        if (DrawRow(pPreview, "Preview")) { m_pPanel_Manager->Clear_Preview(); return; }
    }
    else
        ImGui::TextDisabled("(no model loaded)");

    // 2) Spawned (palette-placed objects, the vector list)
    ImGui::SeparatorText("Spawned");
    CGameObject* pPendingDelete = nullptr;

    auto DrawPartRow = [&](CGameObject* pOwner, const _wstring& strPartTag, CPartObject* pPart)
        {
            if (!pPart)
                return;

            ImGui::PushID(pPart);

            const _bool bHasAnim =
                pPart->Get_Component<CModel>(L"Com_Model") &&
                pPart->Get_Component<CAnimator>(L"Com_Animator");

            string strLabel = ToUtf8(strPartTag);
            if (strLabel.empty())
                strLabel = "Part";

            if (!bHasAnim)
                strLabel += "  (no anim)";

            if (!bHasAnim)
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

            const _bool bSelected = (m_pPanel_Manager->Get_Selected() == pPart);

            if (ImGui::Selectable(strLabel.c_str(), bSelected, ImGuiSelectableFlags_SpanAvailWidth))
            {
                const _wstring strDisplayName = pOwner->Get_ObjectTag() + L"/" + strPartTag;
                m_pPanel_Manager->Bind_ForAnimSource(pOwner, pPart, strDisplayName);
            }

            if (!bHasAnim)
                ImGui::PopStyleColor();

            ImGui::PopID();
        };


    for (auto* pObj : pLevel->Get_SpawnedObjects())
    {
        if (!pObj)
            continue;

        if (auto* pContainer = dynamic_cast<CContainerObject*>(pObj))
        {
            ImGui::PushID(pObj);

            string strName = ToUtf8(pObj->Get_ObjectTag());
            if (strName.empty())
                strName = "Container";

            const _bool bSelected = (m_pPanel_Manager->Get_Selected() == pObj);

            if (ImGui::SmallButton("Delete"))
                pPendingDelete = pObj;
            ImGui::SameLine();

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanAvailWidth;

            if (bSelected)
                flags |= ImGuiTreeNodeFlags_Selected;

            const _bool bOpen = ImGui::TreeNodeEx(strName.c_str(), flags);

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                m_pPanel_Manager->Bind_ForAnimSource(pObj, nullptr, pObj->Get_ObjectTag());

            if (bSelected && ImGui::IsKeyPressed(ImGuiKey_Delete))
                pPendingDelete = pObj;

            if (bOpen)
            {
                const auto& Parts = pContainer->Get_PartObjects();

                if (Parts.empty())
                {
                    ImGui::TextDisabled("(no parts)");
                }
                else
                {
                    vector<pair<_wstring, CPartObject*>> SortedParts;
                    SortedParts.reserve(Parts.size());

                    for (const auto& Pair : Parts)
                    {
                        if (Pair.second)
                            SortedParts.emplace_back(Pair.first, Pair.second);
                    }

                    sort(SortedParts.begin(), SortedParts.end(),
                        [](const auto& Lhs, const auto& Rhs)
                        {
                            return Lhs.first < Rhs.first;
                        });

                    for (const auto& Pair : SortedParts)
                        DrawPartRow(pObj, Pair.first, Pair.second);
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
        else
        {
            if (DrawRow(pObj, "Object"))
                pPendingDelete = pObj;
        }
    }

    if (pPendingDelete)
    {
        CGameObject* pSelected = m_pPanel_Manager->Get_Selected();

        if (pSelected == pPendingDelete)
        {
            m_pPanel_Manager->Clear_Selected();
        }
        else if (auto* pContainer = dynamic_cast<CContainerObject*>(pPendingDelete))
        {
            for (const auto& Pair : pContainer->Get_PartObjects())
            {
                if (pSelected == Pair.second)
                {
                    m_pPanel_Manager->Clear_Selected();
                    break;
                }
            }
        }

        pLevel->Destroy_Spawned(pPendingDelete);
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
    auto& UIContainers = pLevel->Get_UIContainerEntries();

    if (UIContext.bDirty)
        ImGui::TextColored(ImVec4(1.f, 0.75f, 0.25f, 1.f), "Modified");

    if (UIContainers.empty())
    {
        ImGui::TextDisabled("(no ui containers)");
        return;
    }

    CUIContainerObject* pPendingDeleteContainer = nullptr;
    CUIContainerObject* pPartDeleteOwner = nullptr;
    CUICoordinatorContainer* pChildDeleteCoord = nullptr;
    _wstring strChildDeleteTag;
    _wstring strPartDeleteTag;

    for (auto& Entry : UIContainers)
    {
        CUIContainerObject* pContainer = Entry.pContainer;

        if (nullptr == pContainer)
            continue;

        std::string strContainerName = ToUtf8(pContainer->Get_ObjectTag());
        if (strContainerName.empty())
            strContainerName = "UIContainer";

        ImGui::PushID(pContainer);

        if (ImGui::SmallButton("X"))
            pPendingDeleteContainer = pContainer;
        ImGui::SameLine();

        const _bool bContainerSelected =
            UIContext.Selection.pContainer == pContainer &&
            UIContext.Selection.pPart == nullptr;

        ImGuiTreeNodeFlags eNodeFlags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth;

        if (bContainerSelected)
            eNodeFlags |= ImGuiTreeNodeFlags_Selected;

        if (!pContainer->Is_Active())
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.f));

        const _bool bOpened = ImGui::TreeNodeEx(strContainerName.c_str(), eNodeFlags);

        if (!pContainer->Is_Active())
            ImGui::PopStyleColor();

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            m_pPanel_Manager->Set_UISelected(pContainer, nullptr, L"");

        if (bOpened)
        {
            const auto& UIParts = pContainer->Get_UIPartObjects();

            if (UIParts.empty())
            {
                ImGui::TextDisabled("(no parts)");
            }
            else
            {
                vector<pair<_wstring, CUIPartObject*>> SortedParts;
                SortedParts.reserve(UIParts.size());

                for (const auto& Pair : UIParts)
                {
                    if (Pair.second)
                        SortedParts.emplace_back(Pair.first, Pair.second);
                }

                sort(SortedParts.begin(), SortedParts.end(),
                    [pContainer](const auto& L, const auto& R)
                    {
                        CUIPartObject* pLeft = L.second;
                        CUIPartObject* pRight = R.second;

                        const _int iLeftLayer = static_cast<_int>(pLeft->Get_RenderLayer());
                        const _int iRightLayer = static_cast<_int>(pRight->Get_RenderLayer());

                        if (iLeftLayer != iRightLayer)
                            return iLeftLayer < iRightLayer;

                        const _float fLeftZ = pLeft->Get_ZOrder();
                        const _float fRightZ = pRight->Get_ZOrder();

                        constexpr _float fZEqualEpsilon = 0.0001f;

                        if (fLeftZ < fRightZ - fZEqualEpsilon)
                            return true;

                        if (fLeftZ > fRightZ + fZEqualEpsilon)
                            return false;

                        const _int iLeftOrder = pContainer->Get_UIPartOrderIndex(L.first);
                        const _int iRightOrder = pContainer->Get_UIPartOrderIndex(R.first);

                        if (iLeftOrder != iRightOrder)
                            return iLeftOrder < iRightOrder;

                        return L.first < R.first;
                    });

                for (const auto& Pair : SortedParts)
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

                    if (ImGui::SmallButton("X"))
                    {
                        pPartDeleteOwner = pContainer;
                        strPartDeleteTag = strPartTag;
                    }
                    ImGui::SameLine();

                    if (ImGui::Selectable(strPartName.c_str(), bSelected,
                        ImGuiSelectableFlags_SpanAvailWidth))
                        m_pPanel_Manager->Set_UISelected(pContainer, pPart, strPartTag);

                    // Browser 에서 .png/.dds 를 이 파트 행에 드롭 -> 텍스처 실시간 적용
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* pPayload =
                            ImGui::AcceptDragDropPayload(DND_FILE_PATH))
                        {
                            std::string strPath(static_cast<const char*>(pPayload->Data));
                            std::string strExt = filesystem::path(strPath).extension().string();
                            for (auto& c : strExt) c = (char)std::tolower((unsigned char)c);

                            if (auto* pImage = dynamic_cast<CUI_Image*>(pPart))
                            {
                                if (strExt == ".png" || strExt == ".dds")
                                {
                                    _wstring strProtoTag = pLevel->Register_TextureProto(StrToWstr(strPath));

                                    if (!strProtoTag.empty() && SUCCEEDED(pImage->Set_Texture(ETOUI(TOOL_LEVEL::EDIT), strProtoTag)))
                                    {
                                        UIContext.bDirty = true;
                                    }
                                }
                            }
                            else if (auto* pAnim = dynamic_cast<CUI_SpriteAnim*>(pPart))
                            {
                                if (strExt == ".dds")
                                {
                                    _wstring strProtoTag = pLevel->Register_TextureProto(StrToWstr(strPath));

                                    if (!strProtoTag.empty() && SUCCEEDED(pAnim->Set_Texture(ETOUI(TOOL_LEVEL::EDIT), strProtoTag)))
                                    {
                                        UIContext.bDirty = true;
                                    }
                                }
                            }
                            else if (auto* pGauge = dynamic_cast<CUI_GaugeFill*>(pPart))
                            {
                                if (strExt == ".png" || strExt == ".dds")
                                {
                                    _wstring strProtoTag =
                                        pLevel->Register_TextureProto(StrToWstr(strPath));

                                    if (!strProtoTag.empty() &&
                                        SUCCEEDED(pGauge->Set_Texture(ETOUI(TOOL_LEVEL::EDIT), strProtoTag)))
                                    {
                                        UIContext.bDirty = true;
                                    }
                                }
                            }
                            else if (auto* pGauge = dynamic_cast<CUI_Curtain*>(pPart))
                            {
                                if (strExt == ".png" || strExt == ".dds")
                                {
                                    _wstring strProtoTag =
                                        pLevel->Register_TextureProto(StrToWstr(strPath));

                                    if (!strProtoTag.empty() &&
                                        SUCCEEDED(pGauge->Set_Texture(ETOUI(TOOL_LEVEL::EDIT), strProtoTag)))
                                    {
                                        UIContext.bDirty = true;
                                    }
                                }
                            }
                            else if (auto* pGauge = dynamic_cast<CUI_CurtainAnimBase*>(pPart))
                            {
                                if (strExt == ".png" || strExt == ".dds")
                                {
                                    _wstring strProtoTag =
                                        pLevel->Register_TextureProto(StrToWstr(strPath));

                                    if (!strProtoTag.empty() &&
                                        SUCCEEDED(pGauge->Set_Texture(ETOUI(TOOL_LEVEL::EDIT), strProtoTag)))
                                    {
                                        UIContext.bDirty = true;
                                    }
                                }
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

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

            if (auto* pCoord = dynamic_cast<Client::CUICoordinatorContainer*>(pContainer))
            {
                for (const _wstring& childTag : pCoord->Get_ChildOrder())
                {
                    CUIContainerObject* pChild = pCoord->Find_Child(childTag);
                    if (!pChild)
                        continue;

                    ImGui::PushID(pChild);

                    if (ImGui::SmallButton("X"))
                    {
                        pChildDeleteCoord = pCoord;
                        strChildDeleteTag = childTag;
                    }
                    ImGui::SameLine();

                    const _bool bChildSelected =
                        UIContext.Selection.pContainer == pChild &&
                        UIContext.Selection.pPart == nullptr;

                    ImGuiTreeNodeFlags eChildFlags =
                        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (bChildSelected)
                        eChildFlags |= ImGuiTreeNodeFlags_Selected;

                    std::string strChildName = "[child] " + ToUtf8(childTag);
                    const _bool bChildOpen = ImGui::TreeNodeEx(strChildName.c_str(), eChildFlags);

                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                        m_pPanel_Manager->Set_UISelected(pChild, nullptr, L"");

                    if (bChildOpen)
                    {
                        const auto& ChildParts = pChild->Get_UIPartObjects();
                        if (ChildParts.empty())
                        {
                            ImGui::TextDisabled("(no parts)");
                        }
                        else
                        {
                            for (const auto& Pair : ChildParts)
                            {
                                CUIPartObject* pPart = Pair.second;
                                if (!pPart)
                                    continue;

                                const _bool bPartSel =
                                    UIContext.Selection.pContainer == pChild &&
                                    UIContext.Selection.pPart == pPart &&
                                    UIContext.Selection.strPartTag == Pair.first;

                                ImGui::PushID(pPart);

                                if (ImGui::SmallButton("X"))
                                {
                                    pPartDeleteOwner = pChild;
                                    strPartDeleteTag = Pair.first;
                                }
                                ImGui::SameLine();

                                if (ImGui::Selectable(ToUtf8(Pair.first).c_str(), bPartSel,
                                    ImGuiSelectableFlags_SpanAvailWidth))
                                    m_pPanel_Manager->Set_UISelected(pChild, pPart, Pair.first);

                                ImGui::PopID();
                            }
                        }
                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (pPartDeleteOwner)
    {
        if (UIContext.Selection.pContainer == pPartDeleteOwner &&
            UIContext.Selection.strPartTag == strPartDeleteTag)
            m_pPanel_Manager->Set_UISelected(pPartDeleteOwner, nullptr, L"");
    
        pLevel->Remove_UIPart(pPartDeleteOwner, strPartDeleteTag);
        UIContext.bDirty = true;
    }
    
    if (pPendingDeleteContainer)
    {
        if (UIContext.Selection.pContainer == pPendingDeleteContainer)
            m_pPanel_Manager->Clear_UISelected();
    
        pLevel->Delete_UIContainer(pPendingDeleteContainer);
        UIContext.bDirty = true;
    }

    if (pChildDeleteCoord)
    {
        // 선택이 지워질 자식이면 코디네이터로 선택 이동
        if (auto* pDelChild = pChildDeleteCoord->Find_Child(strChildDeleteTag))
            if (UIContext.Selection.pContainer == pDelChild)
                m_pPanel_Manager->Set_UISelected(pChildDeleteCoord, nullptr, L"");

        pChildDeleteCoord->Remove_Child(strChildDeleteTag);
        UIContext.bDirty = true;
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

    case RENDERUIID::CURTAIN:
        return "CURTAIN";

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