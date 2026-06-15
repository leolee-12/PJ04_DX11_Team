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
    for (auto* pObj : pLevel->Get_SpawnedObjects())
        if (pObj && DrawRow(pObj, "Object"))
            pPendingDelete = pObj;

    if (pPendingDelete)
    {
        if (m_pPanel_Manager->Get_Selected() == pPendingDelete)
            m_pPanel_Manager->Clear_Selected();
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