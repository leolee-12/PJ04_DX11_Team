#include "Map_OverrideStore.h"
#include "Map_EditFile.h"

NS_BEGIN(Client)

HRESULT CMap_OverrideStore::Get_OverrideAssetPath(const _wstring& strManifestPath, _wstring*
	pOutOverridePath)
{
	return CMap_EditFile::Get_EditFilePath(strManifestPath, pOutOverridePath);
}

HRESULT CMap_OverrideStore::Load_OverrideAsset(
	const _wstring& strManifestPath,
	MAP_EDIT_DATA* pInOutMapContentDesc,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	return CMap_EditFile::Load_EditFile(
		strManifestPath,
		pInOutMapContentDesc,
		pOutMapStageOverride,
		pOutHasMapStageOverride);
}

HRESULT CMap_OverrideStore::Save_OverrideAsset(const MAP_EDIT_DATA& MapContentDesc, const
	CMapStage* pStage)
{
	return CMap_EditFile::Save_EditFile(MapContentDesc, pStage);
}

HRESULT CMap_OverrideStore::Get_PresetOverrideAssetPath(
	_uint iPresetIndex,
	const _wstring& strManifestPath,
	_wstring* pOutOverridePath)
{
	return CMap_EditFile::Get_PresetEditFilePath(iPresetIndex, strManifestPath, pOutOverridePath);
}

HRESULT CMap_OverrideStore::Load_PresetOverrideAsset(
	_uint iPresetIndex,
	const _wstring& strManifestPath,
	MAP_EDIT_DATA* pInOutMapContentDesc,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	return CMap_EditFile::Load_PresetEditFile(
		iPresetIndex,
		strManifestPath,
		pInOutMapContentDesc,
		pOutMapStageOverride,
		pOutHasMapStageOverride);
}

HRESULT CMap_OverrideStore::Save_PresetOverrideAsset(
	_uint iPresetIndex,
	const MAP_EDIT_DATA& MapContentDesc,
	const CMapStage* pStage)
{
	return CMap_EditFile::Save_PresetEditFile(iPresetIndex, MapContentDesc, pStage);
}

NS_END