#pragma once
#include "Map_LoadTypes.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CMap_EditSession final : public CBase
{
public:
	struct MAP_EDIT_ENV_ITEM
	{
		_wstring strStableKey;
		_wstring strDisplayName;
		_wstring strLayerTag;
		_wstring strObjectTag;
		_wstring wstrSourceFile;
		_wstring wstrSection;
		_wstring wstrEntryKey;
		_uint    iUid = {};
	};

	struct MAP_EDIT_ADDED_ITEM
	{
		_wstring strPrototypeTag;
		_wstring strLayerTag;
		_wstring strObjectTag;
		_wstring strDisplayName;
	};

private:
	CMap_EditSession();
	virtual ~CMap_EditSession() = default;

public:
	void Reset();

	const MAP_EDIT_DATA& Get_EditData() const { return m_tEditData; }
	void Set_EditData(const MAP_EDIT_DATA& Desc);
	void Set_EditMeta(const MAP_EDIT_DATA& Desc);

	_bool Is_LoadStageEnabled() const { return m_tEditData.bLoadStage; }
	_bool Is_LoadEnvEnabled() const { return m_tEditData.bLoadEnv; }
	void Set_LoadStageEnabled(_bool bEnable);
	void Set_LoadEnvEnabled(_bool bEnable);

	void Set_PresetIndex(_int iPresetIndex);
	void Set_ManifestPath(const _wstring& strManifestPath);

	_uint Get_DeletedEnvCount() const { return static_cast<_uint>(m_DeletedMapPreviewEnvOrder.size()); }

	void Register_PreviewObject(const _wstring& strLayerTag, const _wstring& strObjectTag, CGameObject* pObject);

	void Unregister_PreviewObject(CGameObject* pObject);
	_bool Can_DeleteAsEnvOverride(CGameObject* pObject) const;
	_bool Track_DeletedPreviewObject(CGameObject* pObject);

	_bool Track_EditedPreviewObject(CGameObject* pObject, const MAP_ENV_EDITED_DESC& Edit);

	_bool Clear_EditedPreviewObject(CGameObject* pObject);

	_bool Try_GetEditedEnvObject(const _wstring& strStableKey, MAP_ENV_EDITED_DESC* pOutEdit) const;

	_bool Track_EditedMapSection(const _wstring& strSectionKey, const MAP_ENV_EDITED_DESC& Edit);

	_bool Clear_EditedMapSection(const _wstring& strSectionKey);

	_bool Try_GetEditedMapSection(const _wstring& strSectionKey, MAP_ENV_EDITED_DESC* pOutEdit) const;

	_uint Get_EditedMapSectionCount() const { return static_cast<_uint>(m_tEditData.OverrideDesc.EditedMapSections.size()); }

	const MAP_EDIT_CHANGE& Get_Change() const { return m_tEditData.OverrideDesc; }
	void Set_Change(const MAP_EDIT_CHANGE& Desc);
	MAP_EDIT_CHANGE Build_ChangeSnapShot() const;
	MAP_EDIT_DATA Build_EditDataSnapShot() const;

	const vector<_wstring>& Get_DeletedEnvOrder() const { return m_DeletedMapPreviewEnvOrder; }
	_bool Try_GetDeletedEnvItem(const _wstring& strStableKey, MAP_EDIT_ENV_ITEM* pOutItem) const;
	_bool Restore_DeletedEnvItem(const _wstring& strStableKey);
	void Restore_AllDeletedEnvItems();
	void Rebuild_DeletedEnvItems(const vector<ENV_OBJECT_DESC>& DeletedDescs);

	void Clear_RuntimeState();

	_bool Is_StageLoaded() const { return m_bStageLoaded; }
	void Set_PreviewStatus(const _wstring& strStatus);
	void Set_LoadedStageName(const _wstring& strStageName);
	void Clear_LoadedStage();

	_bool Is_EnvLoaded() const { return m_bEnvLoaded; }
	void Set_EnvLoaded(_bool bLoaded);
	void Set_EnvCreatedCount(_uint iCount);

	_bool Is_PreviewLoaded() const { return m_bStageLoaded || m_bEnvLoaded || 0 != m_iEnvCreatedCount; }
	const _wstring& Get_PreviewStatus() const { return m_strPreviewStatus; }
	const _wstring& Get_LoadedStageName() const { return m_strLoadedStageName; }
	_uint Get_EnvCreatedCount() const { return m_iEnvCreatedCount; }

	void Register_AddedObject(
		CGameObject* pObject,
		const MAP_ADD_OBJECT& Desc,
		const _wstring& strDisplayName);

	_bool Unregister_AddedObject(CGameObject* pObject);
	_bool Is_AddedObject(CGameObject* pObject) const;

	_uint Get_AddedObjectCount() const;
	const vector<CGameObject*>& Get_AddedObjectOrder() const;
	_bool Try_GetAddedObjectItem(CGameObject* pObject, MAP_EDIT_ADDED_ITEM* pOutItem) const;

private:
	MAP_EDIT_DATA m_tEditData = {};

	unordered_map<CGameObject*, MAP_EDIT_ENV_ITEM> m_MapPreviewEnvItems;
	unordered_map<_wstring, MAP_EDIT_ENV_ITEM> m_DeletedMapPreviewEnvItems;
	vector<_wstring> m_DeletedMapPreviewEnvOrder;

	unordered_map<CGameObject*, Client::MAP_ADD_OBJECT> m_AddedObjectsByRuntime;
	unordered_map<CGameObject*, MAP_EDIT_ADDED_ITEM> m_AddedObjectUiItems;
	vector<CGameObject*> m_AddedObjectOrder;

	_bool m_bStageLoaded = { false };
	_bool m_bEnvLoaded = { false };
	_wstring m_strPreviewStatus = { L"Map preset not loaded." };
	_wstring m_strLoadedStageName = {};
	_uint m_iEnvCreatedCount = {};

private:
	static _bool Is_PreviewEnvLayer(const _wstring& strLayerTag);
	static _wstring Build_DisplayName(const ENV_OBJECT_DESC& Desc);
	void Rebuild_DeletedEnvItemsFromWorkingDelta();
	_bool Remove_AddedObjectDescByKey(const _wstring& strLayerTag, const _wstring& strObjectTag);

public:
	static CMap_EditSession* Create();

private:
	virtual void Free() override;
};

NS_END
