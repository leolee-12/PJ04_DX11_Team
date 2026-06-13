#include "Map_OverrideStore.h"
#include "Map_PresetCatalog.h"
#include "Map_StageOverrideSerializer.h"

#include "DataExporter.h"
#include "DataLoader.h"

#include <filesystem>

NS_BEGIN(Client)

namespace
{
	using namespace std::filesystem;

	HRESULT Resolve_OverrideAssetPathFromManifest_Impl(
		const _wstring& strManifestPath,
		_wstring* pOutOverridePath)
	{
		if (nullptr == pOutOverridePath || strManifestPath.empty())
			return E_FAIL;

		pOutOverridePath->clear();

		const path ManifestPath = path(strManifestPath).lexically_normal();
		if (ManifestPath.empty())
			return E_FAIL;

		const path OverridePath = ManifestPath.parent_path()
			/ (ManifestPath.stem().wstring() + L".EditorOverride.json");

		*pOutOverridePath = OverridePath.lexically_normal().wstring();
		return S_OK;
	}

	HRESULT Resolve_PresetManifestPathForStore(_uint iPresetIndex, const _wstring& strManifestPath, _wstring* pOutManifestPath)
	{
		if (nullptr == pOutManifestPath)
			return E_FAIL;

		pOutManifestPath->clear();

		if (!strManifestPath.empty())
		{
			*pOutManifestPath = strManifestPath;
			return S_OK;
		}

		return CMap_PresetCatalog::Get_ManifestPath(iPresetIndex, pOutManifestPath);
	}
}

HRESULT CMap_OverrideStore::Get_OverrideAssetPath(const _wstring& strManifestPath, _wstring* pOutOverridePath)
{
	return Resolve_OverrideAssetPathFromManifest_Impl(strManifestPath, pOutOverridePath);
}

HRESULT CMap_OverrideStore::Load_OverrideAsset(
	const _wstring& strManifestPath,
	MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	if (nullptr == pInOutMapContentDesc)
		return E_FAIL;

	if (nullptr != pOutMapStageOverride)
		*pOutMapStageOverride = json::object();

	if (nullptr != pOutHasMapStageOverride)
		*pOutHasMapStageOverride = false;

	if (strManifestPath.empty())
		return E_FAIL;

	if (pInOutMapContentDesc->strManifestPath.empty())
		pInOutMapContentDesc->strManifestPath = strManifestPath;

	_wstring strOverridePath;
	if (FAILED(Resolve_OverrideAssetPathFromManifest_Impl(strManifestPath, &strOverridePath)))
		return E_FAIL;

	string strContent;
	if (FAILED(CDataLoader::Read_Json(strOverridePath.c_str(), &strContent)))
		return S_FALSE;

	try
	{
		const json jRoot = json::parse(strContent);

		MAP_LEVEL_CONTENT_DESC SavedMapContentDesc{};
		if (FAILED(CMap_LevelContent::Deserialize(jRoot, &SavedMapContentDesc)))
			return E_FAIL;

		if (SavedMapContentDesc.bHasMapContent)
		{
			pInOutMapContentDesc->bHasMapContent = true;
			pInOutMapContentDesc->Version = SavedMapContentDesc.Version;

			if (0 <= SavedMapContentDesc.iPresetIndex)
				pInOutMapContentDesc->iPresetIndex = SavedMapContentDesc.iPresetIndex;

			if (!SavedMapContentDesc.strManifestPath.empty())
				pInOutMapContentDesc->strManifestPath = SavedMapContentDesc.strManifestPath;

			pInOutMapContentDesc->bLoadStage = SavedMapContentDesc.bLoadStage;
			pInOutMapContentDesc->bLoadEnv = SavedMapContentDesc.bLoadEnv;
			pInOutMapContentDesc->OverrideDesc = SavedMapContentDesc.OverrideDesc;
		}

		if (nullptr != pOutMapStageOverride
			&& nullptr != pOutHasMapStageOverride
			&& jRoot.contains("MapStage")
			&& jRoot["MapStage"].is_object())
		{
			*pOutMapStageOverride = jRoot["MapStage"];
			*pOutHasMapStageOverride = true;
		}
	}
	catch (json::exception&)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMap_OverrideStore::Save_OverrideAsset(const MAP_LEVEL_CONTENT_DESC& MapContentDesc, const CMapStage* pStage)
{
	if (!MapContentDesc.bHasMapContent || MapContentDesc.strManifestPath.empty())
		return E_FAIL;

	_wstring strOverridePath;
	if (FAILED(Resolve_OverrideAssetPathFromManifest_Impl(
		MapContentDesc.strManifestPath,
		&strOverridePath)))
	{
		return E_FAIL;
	}

	json jRoot = json::object();
	jRoot["MapContent"] = CMap_LevelContent::Serialize(MapContentDesc);

	if (nullptr != pStage)
		jRoot["MapStage"] = CMap_StageOverrideSerializer::Serialize(pStage);

	return CDataExporter::Write_JsonFile(strOverridePath.c_str(), jRoot);
}

HRESULT CMap_OverrideStore::Get_PresetOverrideAssetPath(
	_uint iPresetIndex,
	const _wstring& strManifestPath,
	_wstring* pOutOverridePath)
{
	_wstring strResolvedManifestPath;
	if (FAILED(Resolve_PresetManifestPathForStore(
		iPresetIndex,
		strManifestPath,
		&strResolvedManifestPath)))
	{
		return E_FAIL;
	}

	return Get_OverrideAssetPath(strResolvedManifestPath, pOutOverridePath);
}

HRESULT CMap_OverrideStore::Load_PresetOverrideAsset(
	_uint iPresetIndex,
	const _wstring& strManifestPath,
	MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	if (nullptr == pInOutMapContentDesc)
		return E_FAIL;

	_wstring strResolvedManifestPath;
	if (FAILED(Resolve_PresetManifestPathForStore(
		iPresetIndex,
		strManifestPath,
		&strResolvedManifestPath)))
	{
		return E_FAIL;
	}

	if (pInOutMapContentDesc->strManifestPath.empty())
		pInOutMapContentDesc->strManifestPath = strResolvedManifestPath;

	const HRESULT hr = Load_OverrideAsset(
		strResolvedManifestPath,
		pInOutMapContentDesc,
		pOutMapStageOverride,
		pOutHasMapStageOverride);

	pInOutMapContentDesc->iPresetIndex = static_cast<_int>(iPresetIndex);
	return hr;
}

HRESULT CMap_OverrideStore::Save_PresetOverrideAsset(
	_uint iPresetIndex,
	const MAP_LEVEL_CONTENT_DESC& MapContentDesc,
	const CMapStage* pStage)
{
	MAP_LEVEL_CONTENT_DESC SaveDesc = MapContentDesc;
	SaveDesc.bHasMapContent = true;
	SaveDesc.iPresetIndex = static_cast<_int>(iPresetIndex);

	if (SaveDesc.strManifestPath.empty())
	{
		if (FAILED(Resolve_PresetManifestPathForStore(
			iPresetIndex,
			L"",
			&SaveDesc.strManifestPath)))
		{
			return E_FAIL;
		}
	}

	return Save_OverrideAsset(SaveDesc, pStage);
}

NS_END