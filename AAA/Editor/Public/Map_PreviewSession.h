#pragma once
#include "Base.h"
#include "Editor_Defines.h"
#include "Map_LevelContent.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Editor)

class CMap_PreviewSession final : public CBase
{
public:
    struct MAP_PREVIEW_ENV_ITEM
    {
        _wstring strStableKey;
        _wstring strDisplayName;
        _wstring strLayerTag;
        _wstring strObjectTag;
        _wstring strSourceFile;
        _wstring strSection;
        _wstring strEntryKey;
        _uint    iUid = {};
    };

    struct MAP_PREVIEW_ADDED_ITEM
    {
        _wstring strPrototypeTag;
        _wstring strLayerTag;
        _wstring strObjectTag;
        _wstring strDisplayName;
    };

private:
    CMap_PreviewSession();
    virtual ~CMap_PreviewSession() = default;

public:
    void Reset();

    const MAP_LEVEL_CONTENT_DESC& Get_MapContentDesc() const { return m_MapContentDesc; }
    void Set_MapContentDesc(const MAP_LEVEL_CONTENT_DESC& Desc);

    _bool Is_LoadStageEnabled() const { return m_MapContentDesc.bLoadStage; }
    _bool Is_LoadEnvEnabled() const { return m_MapContentDesc.bLoadEnv; }
    void Set_LoadStageEnabled(_bool bEnable);
    void Set_LoadEnvEnabled(_bool bEnable);

    void Set_PresetIndex(_int iPresetIndex);
    void Set_ManifestPath(const _wstring& strManifestPath);

    _uint Get_DeletedEnvCount() const { return static_cast<_uint>(m_DeletedMapPreviewEnvOrder.size()); }

    void Register_PreviewObject(const _wstring& strLayerTag, const _wstring& strObjectTag, CGameObject* pObject);
    void Unregister_PreviewObject(CGameObject* pObject);
    _bool Track_DeletedPreviewObject(CGameObject* pObject);

    const MAP_OVERRIDE_DESC& Get_WorkingDelta() const { return m_MapContentDesc.OverrideDesc; }
    void Set_WorkingDelta(const MAP_OVERRIDE_DESC& Desc);
    MAP_OVERRIDE_DESC Build_WorkingDeltaSnapshot() const;
    MAP_LEVEL_CONTENT_DESC Build_MapContentSnapshot() const;

    const vector<_wstring>& Get_DeletedEnvOrder() const { return m_DeletedMapPreviewEnvOrder; }
    _bool Try_GetDeletedEnvItem(const _wstring& strStableKey, MAP_PREVIEW_ENV_ITEM* pOutItem) const;
    _bool Restore_DeletedEnvItem(const _wstring& strStableKey);
    void Restore_AllDeletedEnvItems();
    void Rebuild_DeletedEnvItems(const vector<Client::ENV_OBJECT_DESC>& DeletedDescs);

    void Clear_RuntimeState();
    void Set_PreviewStatus(const _wstring& strStatus);
    void Set_LoadedStageName(const _wstring& strStageName);
    void Clear_LoadedStage();
    void Set_EnvCreatedCount(_uint iCount);

    _bool Is_PreviewLoaded() const { return m_bStageLoaded || 0 != m_iEnvCreatedCount; }
    const _wstring& Get_PreviewStatus() const { return m_strPreviewStatus; }
    const _wstring& Get_LoadedStageName() const { return m_strLoadedStageName; }
    _uint Get_EnvCreatedCount() const { return m_iEnvCreatedCount; }

    void Register_AddedMapObject(
        CGameObject* pObject,
        const MAP_ADDED_OBJECT_DESC& Desc,
        const _wstring& strDisplayName);

    _bool Unregister_AddedMapObject(CGameObject* pObject);
    _bool Is_AddedMapObject(CGameObject* pObject) const;

    _uint Get_AddedMapObjectCount() const;
    const vector<CGameObject*>& Get_AddedMapObjectOrder() const;
    _bool Try_GetAddedMapObjectItem(CGameObject* pObject, MAP_PREVIEW_ADDED_ITEM* pOutItem) const;

private:
    MAP_LEVEL_CONTENT_DESC m_MapContentDesc = {};

    unordered_map<CGameObject*, MAP_PREVIEW_ENV_ITEM> m_MapPreviewEnvItems;
    unordered_map<_wstring, MAP_PREVIEW_ENV_ITEM> m_DeletedMapPreviewEnvItems;
    vector<_wstring> m_DeletedMapPreviewEnvOrder;

    unordered_map<CGameObject*, Client::MAP_ADDED_OBJECT_DESC> m_AddedMapObjectsByRuntime;
    unordered_map<CGameObject*, MAP_PREVIEW_ADDED_ITEM> m_AddedMapObjectUiItems;
    vector<CGameObject*> m_AddedMapObjectOrder;

    _bool m_bStageLoaded = false;
    _wstring m_strPreviewStatus = { L"Map preset not loaded." };
    _wstring m_strLoadedStageName = {};
    _uint m_iEnvCreatedCount = {};

private:
    static _bool Is_PreviewEnvLayer(const _wstring& strLayerTag);
    static _wstring Build_DisplayName(const ENV_OBJECT_DESC& Desc);
    void Rebuild_DeletedEnvItemsFromWorkingDelta();

public:
    static CMap_PreviewSession* Create();

private:
    virtual void Free() override;
};

NS_END