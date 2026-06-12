#include "Panel_Hierarchy.h"
#include "EditInstance.h"
#include "Level_Edit.h"
#include "Map_EditSession.h"

#include "GameInstance.h"
#include "GameObject.h"

#include <algorithm>
#include <cctype>

#include "imgui.h"

namespace
{
    string To_LowerAscii(string strText)
    {
        transform(
            strText.begin(),
            strText.end(),
            strText.begin(),
            [](_char ch)
            {
                return static_cast<_char>(tolower(static_cast<unsigned char>(ch)));
            });

        return strText;
    }

    _bool Contains_SearchText(const string& strLowerHaystack, const string& strLowerNeedle)
    {
        if (strLowerNeedle.empty())
            return true;

        return string::npos != strLowerHaystack.find(strLowerNeedle);
    }

    string Build_SearchKeyLower(
        const string& strLayerLower,
        const string& strPrototypeUtf8,
        const string& strNameUtf8)
    {
        string strKey = strLayerLower;
        strKey += " ";
        strKey += To_LowerAscii(strPrototypeUtf8);
        strKey += " ";
        strKey += To_LowerAscii(strNameUtf8);
        return strKey;
    }

    _bool Pass_Search(const string& strHaystack, const string& strLowerNeedle)
    {
        return Contains_SearchText(To_LowerAscii(strHaystack), strLowerNeedle);
    }

    string Make_SearchKey(const string& strLayerName, const MapTool::CLevel_Edit::EDITOR_OBJECT_HANDLE& Handle)
    {
        string strSearchKey = strLayerName;
        strSearchKey += " ";
        strSearchKey += WstrToStr(Handle.strPrototypeTag);
        strSearchKey += " ";
        strSearchKey += WstrToStr(Handle.strName);
        return strSearchKey;
    }

    void Draw_MapPreviewDeletedOverrides(MapTool::CLevel_Edit* pLevel)
    {
        ImGui::TextUnformatted("Deleted Env Overrides");

        const CMap_EditSession* pSession =
            (nullptr != pLevel) ? pLevel->Get_MapPreviewSession() : nullptr;

        if (nullptr == pSession || 0 == pSession->Get_DeletedEnvCount())
        {
            ImGui::TextDisabled("No deleted env overrides.");
            return;
        }

        _wstring strRestoreKey;
        _bool bRestoreAll = false;

        if (ImGui::Button("Restore All"))
            bRestoreAll = true;

        ImGui::BeginChild("DeletedEnvOverrides", ImVec2(0.f, 160.f), true);

        for (const auto& strKey : pSession->Get_DeletedEnvOrder())
        {
            CMap_EditSession::MAP_PREVIEW_ENV_ITEM Item{};
            if (!pSession->Try_GetDeletedEnvItem(strKey, &Item))
                continue;

            const string strKeyUtf8 = WstrToStr(strKey);
            const string strDisplayUtf8 =
                WstrToStr(Item.strDisplayName.empty() ? strKey : Item.strDisplayName);

            const string strLayerUtf8 = WstrToStr(Item.strLayerTag);
            const string strObjectTagUtf8 = WstrToStr(Item.strObjectTag);
            const string strSourceUtf8 = WstrToStr(Item.strSourceFile);
            const string strSectionUtf8 = WstrToStr(Item.strSection);
            const string strEntryUtf8 = WstrToStr(Item.strEntryKey);

            ImGui::PushID(strKeyUtf8.c_str());

            ImGui::TextWrapped("%s", strDisplayUtf8.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", strKeyUtf8.c_str());

            if (!strLayerUtf8.empty() || !strObjectTagUtf8.empty())
            {
                ImGui::TextDisabled(
                    "Layer=%s / Tag=%s",
                    strLayerUtf8.c_str(),
                    strObjectTagUtf8.c_str());
            }

            if (!strSourceUtf8.empty() || !strSectionUtf8.empty() || !strEntryUtf8.empty())
            {
                ImGui::TextDisabled(
                    "File=%s / Section=%s / Entry=%s / Uid=%u",
                    strSourceUtf8.c_str(),
                    strSectionUtf8.c_str(),
                    strEntryUtf8.c_str(),
                    Item.iUid);
            }

            if (ImGui::SmallButton("Restore"))
                strRestoreKey = strKey;

            ImGui::Separator();
            ImGui::PopID();
        }

        ImGui::EndChild();

        if (bRestoreAll)
            pLevel->Restore_AllDeletedMapPreviewEnv();
        else if (!strRestoreKey.empty())
            pLevel->Restore_DeletedMapPreviewEnv(strRestoreKey);
    }

    void Draw_MapPreviewAddedOverrides(MapTool::CLevel_Edit* pLevel)
    {
        ImGui::TextUnformatted("Added Map Overrides");

        const CMap_EditSession* pSession =
            (nullptr != pLevel) ? pLevel->Get_MapPreviewSession() : nullptr;

        if (nullptr == pSession || 0 == pSession->Get_AddedMapObjectCount())
        {
            ImGui::TextDisabled("No added map overrides.");
            return;
        }

        CGameObject* pSelectObject = nullptr;
        CGameObject* pDeleteObject = nullptr;

        ImGui::BeginChild("AddedMapOverrides", ImVec2(0.f, 140.f), true);

        for (CGameObject* pObject : pSession->Get_AddedMapObjectOrder())
        {
            if (nullptr == pObject)
                continue;

            CMap_EditSession::MAP_PREVIEW_ADDED_ITEM Item{};
            if (!pSession->Try_GetAddedMapObjectItem(pObject, &Item))
                continue;

            const _wstring strDisplayName =
                Item.strDisplayName.empty()
                ? (!Item.strObjectTag.empty() ? Item.strObjectTag : Item.strPrototypeTag)
                : Item.strDisplayName;

            const string strDisplayUtf8 = WstrToStr(strDisplayName);
            const string strProtoUtf8 = WstrToStr(Item.strPrototypeTag);
            const string strLayerUtf8 = WstrToStr(Item.strLayerTag);
            const string strObjectTagUtf8 = WstrToStr(Item.strObjectTag);

            ImGui::PushID(pObject);

            ImGui::TextWrapped("%s", strDisplayUtf8.c_str());
            if (!strProtoUtf8.empty() || !strLayerUtf8.empty())
            {
                ImGui::TextDisabled(
                    "Proto=%s / Layer=%s / Tag=%s",
                    strProtoUtf8.c_str(),
                    strLayerUtf8.c_str(),
                    strObjectTagUtf8.c_str());
            }

            if (ImGui::Button("Select"))
                pSelectObject = pObject;

            ImGui::SameLine();
            if (ImGui::SmallButton("Remove"))
                pDeleteObject = pObject;

            ImGui::Separator();
            ImGui::PopID();
        }

        ImGui::EndChild();

        if (nullptr != pSelectObject)
            pLevel->Set_Selected(pSelectObject);

        if (nullptr != pDeleteObject)
            pLevel->Delete_Object(pDeleteObject);
    }
}

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

    const CMap_EditSession* pSession = pLevel->Get_MapPreviewSession();

    const _uint iSelectionRevision = pLevel->Get_SelectionRevision();
    if (m_iCachedSelectionRevision != iSelectionRevision)
    {
        m_iCachedSelectionRevision = iSelectionRevision;
        m_pPendingScrollTarget = pLevel->Get_Selected();
    }

    ImGui::SetNextItemWidth(-70.f);
    _bool bSearchTextChanged = ImGui::InputTextWithHint(
        "##HierarchySearch",
        "Search name / proto / layer",
        m_szSearch,
        SEARCH_BUF_SIZE);

    ImGui::SameLine();
    if (ImGui::SmallButton("Clear") && '\0' != m_szSearch[0])
    {
        memset(m_szSearch, 0, sizeof(m_szSearch));
        bSearchTextChanged = true;
    }

    const string strLowerSearch = To_LowerAscii(m_szSearch);
    const _bool bSearchActive = !strLowerSearch.empty();

    const _uint iPrevHierarchyRevision = m_iCachedHierarchyRevision;
    Sync_HierarchyCache(pLevel);
    const _bool bHierarchyCacheRefreshed =
        (iPrevHierarchyRevision != m_iCachedHierarchyRevision);

    if (bHierarchyCacheRefreshed
        || bSearchTextChanged
        || m_strCachedSearchLower != strLowerSearch
        || (bSearchActive && m_FilterCache.size() != m_HierarchyCache.size()))
    {
        Rebuild_FilterCache(strLowerSearch);
    }

    _bool bHierarchyMutated = false;

    for (_uint iLayer = 0;
        iLayer < static_cast<_uint>(m_HierarchyCache.size()) && !bHierarchyMutated;
        ++iLayer)
    {
        const auto& LayerCache = m_HierarchyCache[iLayer];

        const _bool bLayerHasPendingScrollTarget =
            nullptr != m_pPendingScrollTarget
            && any_of(
                LayerCache.Objects.begin(),
                LayerCache.Objects.end(),
                [&](const HIERARCHY_OBJECT_CACHE& ObjectCache)
                {
                    return ObjectCache.pObject == m_pPendingScrollTarget;
                });

        const HIERARCHY_FILTER_CACHE* pFilterCache = nullptr;
        _uint iMatchedInLayer = static_cast<_uint>(LayerCache.Objects.size());
        _bool bPendingTargetAlreadyMatched = false;

        if (bSearchActive)
        {
            pFilterCache = &m_FilterCache[iLayer];
            iMatchedInLayer =
                static_cast<_uint>(pFilterCache->MatchedObjectIndices.size());

            if (bLayerHasPendingScrollTarget)
            {
                for (const _uint iMatchedObjectIndex : pFilterCache->MatchedObjectIndices)
                {
                    if (LayerCache.Objects[iMatchedObjectIndex].pObject == m_pPendingScrollTarget)
                    {
                        bPendingTargetAlreadyMatched = true;
                        break;
                    }
                }
            }

            if (0 == iMatchedInLayer && !bLayerHasPendingScrollTarget)
                continue;
        }

        const _uint iDisplayCount = bSearchActive
            ? iMatchedInLayer + ((bLayerHasPendingScrollTarget && !bPendingTargetAlreadyMatched) ? 1u : 0u)
            : static_cast<_uint>(LayerCache.Objects.size());

        string strTreeLabel = LayerCache.strLayerNameUtf8;
        if (bSearchActive)
        {
            strTreeLabel += " (" + to_string(iDisplayCount)
                + "/" + to_string(LayerCache.Objects.size()) + ")";
        }

        if (bLayerHasPendingScrollTarget)
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        else if (bSearchActive)
            ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

        ImGui::PushID(LayerCache.strLayerNameUtf8.c_str());

        if (ImGui::TreeNodeEx(
            "##LayerNode",
            ImGuiTreeNodeFlags_SpanAvailWidth,
            "%s",
            strTreeLabel.c_str()))
        {
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("OBJECT_DND"))
                {
                    CGameObject* pDropped = *(CGameObject**)payload->Data;
                    pLevel->Change_ObjectLayer(pDropped, LayerCache.strLayerTag);
                    bHierarchyMutated = true;
                }

                ImGui::EndDragDropTarget();
            }

            if (!bHierarchyMutated)
            {
                _bool bPendingScrollTargetDrawn = false;

                auto DrawObjectRow =
                    [&](const HIERARCHY_OBJECT_CACHE& ObjectCache) -> _bool
                    {
                        CGameObject* pObject = ObjectCache.pObject;
                        if (nullptr == pObject)
                            return false;

                        const _bool bSelected = (pObject == pLevel->Get_Selected());
                        const _bool bIsMapPreviewObject =
                            pLevel->Is_MapPreviewObject(pObject);
                        const _bool bIsAddedMapOverride =
                            (nullptr != pSession) && pSession->Is_AddedMapObject(pObject);

                        string strDisplayName = ObjectCache.strNameUtf8;
                        if (bIsAddedMapOverride)
                            strDisplayName += " [Added]";
                        else if (bIsMapPreviewObject)
                            strDisplayName += " [Preview]";

                        const float fAvailWidth = ImGui::GetContentRegionAvail().x;

                        ImGui::PushID(pObject);

                        if (ImGui::Selectable(
                            strDisplayName.c_str(),
                            bSelected,
                            0,
                            ImVec2(max(1.f, fAvailWidth - 25.f), 0.f)))
                        {
                            pLevel->Set_Selected(pObject);
                        }

                        if (pObject == m_pPendingScrollTarget)
                        {
                            ImGui::SetScrollHereY(0.25f);
                            m_pPendingScrollTarget = nullptr;
                            bPendingScrollTargetDrawn = true;
                        }

                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                        {
                            if (bIsAddedMapOverride && nullptr != pSession)
                            {
                                CMap_EditSession::MAP_PREVIEW_ADDED_ITEM Item{};
                                if (pSession->Try_GetAddedMapObjectItem(pObject, &Item))
                                {
                                    const string strProtoUtf8 =
                                        WstrToStr(Item.strPrototypeTag);
                                    const string strLayerUtf8 =
                                        WstrToStr(Item.strLayerTag);
                                    const string strObjectTagUtf8 =
                                        WstrToStr(Item.strObjectTag);

                                    ImGui::SetTooltip(
                                        "Added by map override.\nProto=%s\nLayer=%s\nTag=%s",
                                        strProtoUtf8.c_str(),
                                        strLayerUtf8.c_str(),
                                        strObjectTagUtf8.c_str());
                                }
                                else
                                {
                                    ImGui::SetTooltip("Added by map override.");
                                }
                            }
                            else if (bIsMapPreviewObject)
                            {
                                ImGui::SetTooltip("Loaded by map preview.");
                            }
                        }

                        if (!bIsMapPreviewObject
                            && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                        {
                            ImGui::SetDragDropPayload(
                                "OBJECT_DND",
                                &pObject,
                                sizeof(CGameObject*));
                            ImGui::Text("%s", strDisplayName.c_str());
                            ImGui::EndDragDropSource();
                        }

                        ImGui::SameLine();

                        if (ImGui::SmallButton("X"))
                        {
                            pLevel->Delete_Object(pObject);
                            bHierarchyMutated = true;
                            ImGui::PopID();
                            return true;
                        }

                        ImGui::PopID();
                        return false;
                    };

                if (bSearchActive)
                {
                    for (_uint iMatchedObjectIndex : pFilterCache->MatchedObjectIndices)
                    {
                        if (DrawObjectRow(LayerCache.Objects[iMatchedObjectIndex]))
                            break;
                    }

                    if (!bHierarchyMutated
                        && bLayerHasPendingScrollTarget
                        && !bPendingScrollTargetDrawn)
                    {
                        for (const auto& ObjectCache : LayerCache.Objects)
                        {
                            if (ObjectCache.pObject != m_pPendingScrollTarget)
                                continue;

                            DrawObjectRow(ObjectCache);
                            break;
                        }
                    }
                }
                else
                {
                    for (const auto& ObjectCache : LayerCache.Objects)
                    {
                        if (DrawObjectRow(ObjectCache))
                            break;
                    }
                }
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (bHierarchyMutated)
    {
        Invalidate_SearchCache();
        End_Panel();
        return;
    }

    if (bSearchActive)
    {
        ImGui::Separator();
        ImGui::TextDisabled(
            "Matched %u objects in %u layers.",
            m_iCachedMatchedObjectCount,
            m_iCachedVisibleLayerCount);

        if (0 == m_iCachedMatchedObjectCount)
            ImGui::TextDisabled("No hierarchy object matched the search.");
    }

    ImGui::Separator();
    Draw_MapPreviewDeletedOverrides(pLevel);

    ImGui::Separator();
    Draw_MapPreviewAddedOverrides(pLevel);

    End_Panel();
}

void CPanel_Hierarchy::Invalidate_SearchCache()
{
    m_strCachedSearchLower.clear();
    m_FilterCache.clear();
    m_iCachedVisibleLayerCount = 0;
    m_iCachedMatchedObjectCount = 0;
}

void CPanel_Hierarchy::Sync_HierarchyCache(CLevel_Edit* pLevel)
{
    if (nullptr == pLevel)
        return;

    const _uint iHierarchyRevision = pLevel->Get_HierarchyRevision();
    if (m_iCachedHierarchyRevision == iHierarchyRevision)
        return;

    m_HierarchyCache.clear();

    const auto& Layers = pLevel->Get_Layers();
    m_HierarchyCache.reserve(Layers.size());

    for (const auto& [LayerTag, Objects] : Layers)
    {
        HIERARCHY_LAYER_CACHE LayerCache{};
        LayerCache.strLayerTag = LayerTag;
        LayerCache.strLayerNameUtf8 = WstrToStr(LayerTag);
        LayerCache.strLayerNameLower = To_LowerAscii(LayerCache.strLayerNameUtf8);
        LayerCache.Objects.reserve(Objects.size());

        for (const auto& Handle : Objects)
        {
            HIERARCHY_OBJECT_CACHE ObjectCache{};
            ObjectCache.strPrototypeUtf8 = WstrToStr(Handle.strPrototypeTag);
            ObjectCache.strNameUtf8 = WstrToStr(Handle.strName);
            ObjectCache.strSearchKeyLower = Build_SearchKeyLower(
                LayerCache.strLayerNameLower,
                ObjectCache.strPrototypeUtf8,
                ObjectCache.strNameUtf8);
            ObjectCache.pObject = Handle.pObject;

            LayerCache.Objects.push_back(ObjectCache);
        }

        m_HierarchyCache.push_back(LayerCache);
    }

    m_iCachedHierarchyRevision = iHierarchyRevision;
    Invalidate_SearchCache();
}

void CPanel_Hierarchy::Rebuild_FilterCache(const string& strLowerSearch)
{
    m_strCachedSearchLower = strLowerSearch;
    m_FilterCache.clear();
    m_FilterCache.resize(m_HierarchyCache.size());

    m_iCachedVisibleLayerCount = 0;
    m_iCachedMatchedObjectCount = 0;

    if (strLowerSearch.empty())
    {
        for (const auto& LayerCache : m_HierarchyCache)
        {
            ++m_iCachedVisibleLayerCount;
            m_iCachedMatchedObjectCount += static_cast<_uint>(LayerCache.Objects.size());
        }

        return;
    }

    for (_uint iLayer = 0; iLayer < static_cast<_uint>(m_HierarchyCache.size()); ++iLayer)
    {
        const auto& LayerCache = m_HierarchyCache[iLayer];
        auto& FilterCache = m_FilterCache[iLayer];

        const _bool bLayerMatched =
            Contains_SearchText(LayerCache.strLayerNameLower, strLowerSearch);

        FilterCache.MatchedObjectIndices.reserve(LayerCache.Objects.size());

        if (bLayerMatched)
        {
            for (_uint iObject = 0; iObject < static_cast<_uint>(LayerCache.Objects.size()); ++iObject)
                FilterCache.MatchedObjectIndices.push_back(iObject);
        }
        else
        {
            for (_uint iObject = 0; iObject < static_cast<_uint>(LayerCache.Objects.size()); ++iObject)
            {
                if (Contains_SearchText(
                    LayerCache.Objects[iObject].strSearchKeyLower,
                    strLowerSearch))
                {
                    FilterCache.MatchedObjectIndices.push_back(iObject);
                }
            }
        }

        if (FilterCache.MatchedObjectIndices.empty())
            continue;

        ++m_iCachedVisibleLayerCount;
        m_iCachedMatchedObjectCount += static_cast<_uint>(FilterCache.MatchedObjectIndices.size());
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