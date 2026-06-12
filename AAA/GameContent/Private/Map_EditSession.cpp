#include "Map_EditSession.h"
#include "EnvObject.h"

#include <algorithm>

NS_BEGIN(Client)

CMap_EditSession::CMap_EditSession()
{
}

void CMap_EditSession::Reset()
{
	m_MapContentDesc = {};
	m_MapPreviewEnvItems.clear();
	m_DeletedMapPreviewEnvItems.clear();
	m_DeletedMapPreviewEnvOrder.clear();
	m_AddedMapObjectsByRuntime.clear();
	m_AddedMapObjectUiItems.clear();
	m_AddedMapObjectOrder.clear();

	Clear_RuntimeState();
}

void CMap_EditSession::Set_MapContentDesc(const MAP_LEVEL_CONTENT_DESC& Desc)
{
	m_MapContentDesc = Desc;
	m_MapContentDesc.bHasMapContent = Desc.bHasMapContent
		|| 0 <= Desc.iPresetIndex
		|| !Desc.strManifestPath.empty();

	m_MapPreviewEnvItems.clear();
	m_AddedMapObjectsByRuntime.clear();
	m_AddedMapObjectUiItems.clear();
	m_AddedMapObjectOrder.clear();

	Rebuild_DeletedEnvItemsFromWorkingDelta();
}

void CMap_EditSession::Set_MapContentMeta(const MAP_LEVEL_CONTENT_DESC& Desc)
{
	m_MapContentDesc = Desc;
	m_MapContentDesc.bHasMapContent = Desc.bHasMapContent
		|| 0 <= Desc.iPresetIndex
		|| !Desc.strManifestPath.empty();
}

void CMap_EditSession::Set_LoadStageEnabled(_bool bEnable)
{
	m_MapContentDesc.bLoadStage = bEnable;
	m_MapContentDesc.bHasMapContent = (0 <= m_MapContentDesc.iPresetIndex)
		|| !m_MapContentDesc.strManifestPath.empty();
}

void CMap_EditSession::Set_LoadEnvEnabled(_bool bEnable)
{
	m_MapContentDesc.bLoadEnv = bEnable;
	m_MapContentDesc.bHasMapContent = (0 <= m_MapContentDesc.iPresetIndex)
		|| !m_MapContentDesc.strManifestPath.empty();
}

void CMap_EditSession::Set_PresetIndex(_int iPresetIndex)
{
	m_MapContentDesc.iPresetIndex = iPresetIndex;
	m_MapContentDesc.bHasMapContent = (0 <= m_MapContentDesc.iPresetIndex)
		|| !m_MapContentDesc.strManifestPath.empty();
}

void CMap_EditSession::Set_ManifestPath(const _wstring& strManifestPath)
{
	m_MapContentDesc.strManifestPath = strManifestPath;
	m_MapContentDesc.bHasMapContent = (0 <= m_MapContentDesc.iPresetIndex)
		|| !m_MapContentDesc.strManifestPath.empty();
}

_bool CMap_EditSession::Is_PreviewEnvLayer(const _wstring& strLayerTag)
{
	return strLayerTag == L"Layer_EnvStatic"
		|| strLayerTag == L"Layer_EnvInteract"
		|| strLayerTag == L"Layer_EnvEffect";
}

_wstring CMap_EditSession::Build_DisplayName(const ENV_OBJECT_DESC& Desc)
{
	_wstring strDisplayName = Desc.strObjectName;

	if (strDisplayName.empty())
	{
		strDisplayName = Desc.strSection;
		if (!Desc.strEntryKey.empty())
		{
			if (!strDisplayName.empty())
				strDisplayName += L":";
			strDisplayName += Desc.strEntryKey;
		}
	}

	if (strDisplayName.empty())
		strDisplayName = L"(EnvObject)";

	strDisplayName += L" / Uid=" + to_wstring(Desc.iUid);
	return strDisplayName;
}

void CMap_EditSession::Register_PreviewObject(
	const _wstring& strLayerTag,
	const _wstring& strObjectTag,
	CGameObject* pObject)
{
	if (nullptr == pObject)
		return;

	if (!Is_PreviewEnvLayer(strLayerTag))
		return;

	CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject);
	if (nullptr == pEnvObject)
		return;

	const ENV_OBJECT_DESC& Desc = pEnvObject->Get_Desc();

	MAP_PREVIEW_ENV_ITEM Item{};
	Item.strStableKey = CMap_Override::Build_EnvObjectStableKey(Desc);
	Item.strDisplayName = Build_DisplayName(Desc);
	if (Item.strDisplayName.empty())
		Item.strDisplayName = strObjectTag;

	Item.strLayerTag = strLayerTag;
	Item.strObjectTag = strObjectTag;
	Item.strSourceFile = Desc.strSourceFile;
	Item.strSection = Desc.strSection;
	Item.strEntryKey = Desc.strEntryKey;
	Item.iUid = Desc.iUid;

	m_MapPreviewEnvItems[pObject] = Item;
}

void CMap_EditSession::Unregister_PreviewObject(CGameObject* pObject)
{
	if (nullptr == pObject)
		return;

	m_MapPreviewEnvItems.erase(pObject);
}

_bool CMap_EditSession::Track_DeletedPreviewObject(CGameObject* pObject)
{
	if (nullptr == pObject)
		return false;

	auto Iter = m_MapPreviewEnvItems.find(pObject);
	if (Iter == m_MapPreviewEnvItems.end())
		return false;

	const MAP_PREVIEW_ENV_ITEM Item = Iter->second;
	m_MapPreviewEnvItems.erase(Iter);

	if (Item.strStableKey.empty())
		return false;

	m_MapContentDesc.OverrideDesc.DeletedEnvObjectKeys.insert(Item.strStableKey);
	m_DeletedMapPreviewEnvItems[Item.strStableKey] = Item;

	const auto OrderIter = find(
		m_DeletedMapPreviewEnvOrder.begin(),
		m_DeletedMapPreviewEnvOrder.end(),
		Item.strStableKey);

	if (OrderIter == m_DeletedMapPreviewEnvOrder.end())
		m_DeletedMapPreviewEnvOrder.push_back(Item.strStableKey);

	return true;
}

_bool CMap_EditSession::Try_GetDeletedEnvItem(
	const _wstring& strStableKey,
	MAP_PREVIEW_ENV_ITEM* pOutItem) const
{
	if (nullptr == pOutItem)
		return false;

	const auto Iter = m_DeletedMapPreviewEnvItems.find(strStableKey);
	if (Iter == m_DeletedMapPreviewEnvItems.end())
		return false;

	*pOutItem = Iter->second;
	return true;
}

_bool CMap_EditSession::Restore_DeletedEnvItem(const _wstring& strStableKey)
{
	if (strStableKey.empty())
		return false;

	const size_t iErased = m_MapContentDesc.OverrideDesc.DeletedEnvObjectKeys.erase(strStableKey);
	m_DeletedMapPreviewEnvItems.erase(strStableKey);
	m_DeletedMapPreviewEnvOrder.erase(
		remove(m_DeletedMapPreviewEnvOrder.begin(), m_DeletedMapPreviewEnvOrder.end(), strStableKey),
		m_DeletedMapPreviewEnvOrder.end());

	return 0 < iErased;
}

void CMap_EditSession::Restore_AllDeletedEnvItems()
{
	m_MapContentDesc.OverrideDesc.DeletedEnvObjectKeys.clear();
	m_DeletedMapPreviewEnvItems.clear();
	m_DeletedMapPreviewEnvOrder.clear();
}

void CMap_EditSession::Rebuild_DeletedEnvItems(const vector<ENV_OBJECT_DESC>& DeletedDescs)
{
	m_DeletedMapPreviewEnvItems.clear();
	m_DeletedMapPreviewEnvOrder.clear();

	unordered_set<_wstring> SeenKeys;
	for (const auto& Desc : DeletedDescs)
	{
		MAP_PREVIEW_ENV_ITEM Item{};
		Item.strStableKey = CMap_Override::Build_EnvObjectStableKey(Desc);
		Item.strDisplayName = Build_DisplayName(Desc);
		Item.strLayerTag = Desc.tRender.strLayerName;
		Item.strSourceFile = Desc.strSourceFile;
		Item.strSection = Desc.strSection;
		Item.strEntryKey = Desc.strEntryKey;
		Item.iUid = Desc.iUid;

		m_DeletedMapPreviewEnvItems[Item.strStableKey] = Item;
		m_DeletedMapPreviewEnvOrder.push_back(Item.strStableKey);
		SeenKeys.insert(Item.strStableKey);
	}

	for (const auto& strKey : m_MapContentDesc.OverrideDesc.DeletedEnvObjectKeys)
	{
		if (SeenKeys.find(strKey) != SeenKeys.end())
			continue;

		MAP_PREVIEW_ENV_ITEM Item{};
		Item.strStableKey = strKey;
		Item.strDisplayName = L"(Unresolved Override)";
		m_DeletedMapPreviewEnvItems[strKey] = Item;
		m_DeletedMapPreviewEnvOrder.push_back(strKey);
	}
}

void CMap_EditSession::Rebuild_DeletedEnvItemsFromWorkingDelta()
{
	m_DeletedMapPreviewEnvItems.clear();
	m_DeletedMapPreviewEnvOrder.clear();

	vector<_wstring> SortedKeys(
		m_MapContentDesc.OverrideDesc.DeletedEnvObjectKeys.begin(),
		m_MapContentDesc.OverrideDesc.DeletedEnvObjectKeys.end());

	sort(SortedKeys.begin(), SortedKeys.end());

	for (const auto& strKey : SortedKeys)
	{
		MAP_PREVIEW_ENV_ITEM Item{};
		Item.strStableKey = strKey;
		Item.strDisplayName = L"(Unresolved Override)";

		m_DeletedMapPreviewEnvItems[strKey] = Item;
		m_DeletedMapPreviewEnvOrder.push_back(strKey);
	}
}

void CMap_EditSession::Clear_RuntimeState()
{
	m_bStageLoaded = false;
	m_bEnvLoaded = false;
	m_strPreviewStatus = L"Map preset not loaded.";
	m_strLoadedStageName.clear();
	m_iEnvCreatedCount = 0;
}

void CMap_EditSession::Set_PreviewStatus(const _wstring& strStatus)
{
	m_strPreviewStatus = strStatus;
}

void CMap_EditSession::Set_LoadedStageName(const _wstring& strStageName)
{
	m_bStageLoaded = true;
	m_strLoadedStageName = strStageName;
}

void CMap_EditSession::Clear_LoadedStage()
{
	m_bStageLoaded = false;
	m_strLoadedStageName.clear();
}

void CMap_EditSession::Set_EnvLoaded(_bool bLoaded)
{
	m_bEnvLoaded = bLoaded;
}

void CMap_EditSession::Set_EnvCreatedCount(_uint iCount)
{
	m_iEnvCreatedCount = iCount;
}

void CMap_EditSession::Set_WorkingDelta(const MAP_OVERRIDE_DESC& Desc)
{
	m_MapContentDesc.OverrideDesc = Desc;
	Rebuild_DeletedEnvItemsFromWorkingDelta();

	m_AddedMapObjectsByRuntime.clear();
	m_AddedMapObjectUiItems.clear();
	m_AddedMapObjectOrder.clear();
}

void CMap_EditSession::Register_AddedMapObject(
	CGameObject* pObject,
	const MAP_ADDED_OBJECT_DESC& Desc,
	const _wstring& strDisplayName)
{
	if (nullptr == pObject)
		return;

	m_AddedMapObjectsByRuntime[pObject] = Desc;

	MAP_PREVIEW_ADDED_ITEM Item{};
	Item.strPrototypeTag = Desc.strPrototypeTag;
	Item.strLayerTag = Desc.strLayerTag;
	Item.strObjectTag = Desc.strObjectTag;
	Item.strDisplayName = strDisplayName;

	m_AddedMapObjectUiItems[pObject] = Item;

	if (find(m_AddedMapObjectOrder.begin(), m_AddedMapObjectOrder.end(), pObject)
		== m_AddedMapObjectOrder.end())
	{
		m_AddedMapObjectOrder.push_back(pObject);
	}
}

_bool CMap_EditSession::Unregister_AddedMapObject(CGameObject* pObject)
{
	if (nullptr == pObject)
		return false;

	_bool bRemoved = false;

	if (0 < m_AddedMapObjectsByRuntime.erase(pObject))
		bRemoved = true;

	if (0 < m_AddedMapObjectUiItems.erase(pObject))
		bRemoved = true;

	auto Iter = find(m_AddedMapObjectOrder.begin(), m_AddedMapObjectOrder.end(), pObject);
	if (Iter != m_AddedMapObjectOrder.end())
	{
		m_AddedMapObjectOrder.erase(Iter);
		bRemoved = true;
	}

	return bRemoved;
}

_bool CMap_EditSession::Is_AddedMapObject(CGameObject* pObject) const
{
	if (nullptr == pObject)
		return false;

	return m_AddedMapObjectsByRuntime.find(pObject) != m_AddedMapObjectsByRuntime.end();
}

_uint CMap_EditSession::Get_AddedMapObjectCount() const
{
	return static_cast<_uint>(m_AddedMapObjectOrder.size());
}

const vector<CGameObject*>& CMap_EditSession::Get_AddedMapObjectOrder() const
{
	return m_AddedMapObjectOrder;
}

_bool CMap_EditSession::Try_GetAddedMapObjectItem(
	CGameObject* pObject,
	MAP_PREVIEW_ADDED_ITEM* pOutItem) const
{
	if (nullptr == pObject || nullptr == pOutItem)
		return false;

	const auto Iter = m_AddedMapObjectUiItems.find(pObject);
	if (Iter == m_AddedMapObjectUiItems.end())
		return false;

	*pOutItem = Iter->second;
	return true;
}

MAP_OVERRIDE_DESC CMap_EditSession::Build_WorkingDeltaSnapshot() const
{
	MAP_OVERRIDE_DESC Snapshot = m_MapContentDesc.OverrideDesc;
	Snapshot.AddedMapObjects.clear();

	if (!m_MapContentDesc.bLoadEnv)
	{
		Snapshot.AddedMapObjects = m_MapContentDesc.OverrideDesc.AddedMapObjects;
		return Snapshot;
	}

	for (CGameObject* pObject : m_AddedMapObjectOrder)
	{
		const auto Iter = m_AddedMapObjectsByRuntime.find(pObject);
		if (Iter == m_AddedMapObjectsByRuntime.end())
			continue;

		MAP_ADDED_OBJECT_DESC AddedDesc = Iter->second;

		if (nullptr != pObject)
			AddedDesc.jObject = pObject->Serialize();

		const auto UiIter = m_AddedMapObjectUiItems.find(pObject);
		if (UiIter != m_AddedMapObjectUiItems.end())
		{
			if (!UiIter->second.strPrototypeTag.empty())
				AddedDesc.strPrototypeTag = UiIter->second.strPrototypeTag;

			if (!UiIter->second.strLayerTag.empty())
				AddedDesc.strLayerTag = UiIter->second.strLayerTag;

			if (!UiIter->second.strObjectTag.empty())
				AddedDesc.strObjectTag = UiIter->second.strObjectTag;
		}

		if (AddedDesc.strPrototypeTag.empty()
			|| AddedDesc.strLayerTag.empty()
			|| AddedDesc.strObjectTag.empty())
		{
			continue;
		}

		Snapshot.AddedMapObjects.push_back(AddedDesc);
	}

	return Snapshot;
}

MAP_LEVEL_CONTENT_DESC CMap_EditSession::Build_MapContentSnapshot() const
{
	MAP_LEVEL_CONTENT_DESC Desc = m_MapContentDesc;
	Desc.bHasMapContent = (0 <= Desc.iPresetIndex) || !Desc.strManifestPath.empty();
	Desc.OverrideDesc = Build_WorkingDeltaSnapshot();
	return Desc;
}

CMap_EditSession* CMap_EditSession::Create()
{
	return new CMap_EditSession();
}

void CMap_EditSession::Free()
{
	__super::Free();
}

NS_END