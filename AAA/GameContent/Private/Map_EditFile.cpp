#include "Map_EditFile.h"
#include "Map_PresetCatalog.h"
#include "MapStage.h"
#include "MapSection.h"

#include "DataExporter.h"
#include "DataLoader.h"

#include <algorithm>
#include <filesystem>

NS_BEGIN(Client)

namespace
{
	using namespace std::filesystem;

	HRESULT Resolve_EditFilePathFromManifest_Impl(
		const _wstring& strManifestPath,
		_wstring* pOutEditFilePath)
	{
		if (nullptr == pOutEditFilePath || strManifestPath.empty())
			return E_FAIL;

		pOutEditFilePath->clear();

		const path ManifestPath = path(strManifestPath).lexically_normal();
		if (ManifestPath.empty())
			return E_FAIL;

		const path EditFilePath = ManifestPath.parent_path()
			/ (ManifestPath.stem().wstring() + L".EditorOverride.json");

		*pOutEditFilePath = EditFilePath.lexically_normal().wstring();
		return S_OK;
	}

	HRESULT Resolve_PresetManifestPathForEditFile(
		_uint iPresetIndex,
		const _wstring& strManifestPath,
		_wstring* pOutManifestPath)
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

_wstring CMap_EditFile::Make_EnvKey(const ENV_OBJECT_DESC& Desc)
{
	return Desc.wstrSourceFile + L"|"
		+ Desc.wstrSection + L"|"
		+ Desc.wstrEntryKey + L"|"
		+ to_wstring(Desc.iUid);
}

HRESULT CMap_EditFile::Apply_Change(
	MAP_PACKAGE* pInOutPackage,
	const MAP_EDIT_CHANGE& OverrideDesc)
{
	if (nullptr == pInOutPackage)
		return E_FAIL;

	if (!OverrideDesc.DeletedEnvObjectKeys.empty())
	{
		auto& EnvObjectDescs = pInOutPackage->EnvObjectDescs;
		EnvObjectDescs.erase(
			remove_if(
				EnvObjectDescs.begin(),
				EnvObjectDescs.end(),
				[&](const ENV_OBJECT_DESC& Desc)
				{
					const _wstring strKey = Make_EnvKey(Desc);
					return OverrideDesc.DeletedEnvObjectKeys.find(strKey)
						!= OverrideDesc.DeletedEnvObjectKeys.end();
				}),
			EnvObjectDescs.end());
	}

	if (!OverrideDesc.AddedMapObjects.empty())
	{
		pInOutPackage->AddedObjectDescs.insert(
			pInOutPackage->AddedObjectDescs.end(),
			OverrideDesc.AddedMapObjects.begin(),
			OverrideDesc.AddedMapObjects.end());
	}

	return S_OK;
}

HRESULT CMap_EditFile::Get_EditFilePath(
	const _wstring& strManifestPath,
	_wstring* pOutEditFilePath)
{
	return Resolve_EditFilePathFromManifest_Impl(strManifestPath, pOutEditFilePath);
}

HRESULT CMap_EditFile::Get_PresetEditFilePath(
	_uint iPresetIndex,
	const _wstring& strManifestPath,
	_wstring* pOutEditFilePath)
{
	_wstring strResolvedManifestPath;
	if (FAILED(Resolve_PresetManifestPathForEditFile(
		iPresetIndex,
		strManifestPath,
		&strResolvedManifestPath)))
	{
		return E_FAIL;
	}

	return Get_EditFilePath(strResolvedManifestPath, pOutEditFilePath);
}

HRESULT CMap_EditFile::Load_EditFile(
	const _wstring& strManifestPath,
	MAP_EDIT_DATA* pInOutMapContentDesc,
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

	_wstring strEditFilePath;
	if (FAILED(Get_EditFilePath(strManifestPath, &strEditFilePath)))
		return E_FAIL;

	string strContent;
	if (FAILED(CDataLoader::Read_Json(strEditFilePath.c_str(), &strContent)))
		return S_FALSE;

	try
	{
		const json jRoot = json::parse(strContent);

		MAP_EDIT_DATA SavedMapContentDesc{};
		if (FAILED(Load_Data(jRoot, &SavedMapContentDesc)))
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
	catch (const json::exception&)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMap_EditFile::Load_PresetEditFile(
	_uint iPresetIndex,
	const _wstring& strManifestPath,
	MAP_EDIT_DATA* pInOutMapContentDesc,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	if (nullptr == pInOutMapContentDesc)
		return E_FAIL;

	_wstring strResolvedManifestPath;
	if (FAILED(Resolve_PresetManifestPathForEditFile(
		iPresetIndex,
		strManifestPath,
		&strResolvedManifestPath)))
	{
		return E_FAIL;
	}

	if (pInOutMapContentDesc->strManifestPath.empty())
		pInOutMapContentDesc->strManifestPath = strResolvedManifestPath;

	const HRESULT hr = Load_EditFile(
		strResolvedManifestPath,
		pInOutMapContentDesc,
		pOutMapStageOverride,
		pOutHasMapStageOverride);

	pInOutMapContentDesc->iPresetIndex = static_cast<_int>(iPresetIndex);
	return hr;
}

HRESULT CMap_EditFile::Save_EditFile(
	const MAP_EDIT_DATA& MapContentDesc,
	const CMapStage* pStage)
{
	if (!MapContentDesc.bHasMapContent || MapContentDesc.strManifestPath.empty())
		return E_FAIL;

	_wstring strEditFilePath;
	if (FAILED(Get_EditFilePath(MapContentDesc.strManifestPath, &strEditFilePath)))
		return E_FAIL;

	json jRoot = json::object();
	jRoot["MapContent"] = Save_Data(MapContentDesc);

	if (nullptr != pStage)
		jRoot["MapStage"] = Save_Stage(pStage);

	return CDataExporter::Write_JsonFile(strEditFilePath.c_str(), jRoot);
}

HRESULT CMap_EditFile::Save_PresetEditFile(
	_uint iPresetIndex,
	const MAP_EDIT_DATA& MapContentDesc,
	const CMapStage* pStage)
{
	MAP_EDIT_DATA SaveDesc = MapContentDesc;
	SaveDesc.bHasMapContent = true;
	SaveDesc.iPresetIndex = static_cast<_int>(iPresetIndex);

	if (SaveDesc.strManifestPath.empty())
	{
		if (FAILED(Resolve_PresetManifestPathForEditFile(
			iPresetIndex,
			L"",
			&SaveDesc.strManifestPath)))
		{
			return E_FAIL;
		}
	}

	return Save_EditFile(SaveDesc, pStage);
}

json CMap_EditFile::Save_Data(const MAP_EDIT_DATA& Desc)
{
	json jMapContent = json::object();

	jMapContent["Version"] = Desc.Version;

	if (0 <= Desc.iPresetIndex)
		jMapContent["PresetIndex"] = Desc.iPresetIndex;

	if (!Desc.strManifestPath.empty())
		jMapContent["Manifest"] = WstrToStr(Desc.strManifestPath);

	jMapContent["LoadStage"] = static_cast<bool>(Desc.bLoadStage);
	jMapContent["LoadEnv"] = static_cast<bool>(Desc.bLoadEnv);
	jMapContent["Override"] = Save_Change(Desc.OverrideDesc);

	return jMapContent;
}

HRESULT CMap_EditFile::Load_Data(const json& jLevelRoot, MAP_EDIT_DATA* pOutDesc)
{
	if (nullptr == pOutDesc)
		return E_FAIL;

	*pOutDesc = {};

	if (!jLevelRoot.is_object())
		return E_FAIL;

	const auto IterMapContent = jLevelRoot.find("MapContent");
	if (IterMapContent == jLevelRoot.end())
		return S_OK;

	if (!IterMapContent->is_object())
		return E_FAIL;

	pOutDesc->bHasMapContent = true;

	const json& jMapContent = *IterMapContent;

	const auto IterVersion = jMapContent.find("Version");
	if (IterVersion != jMapContent.end() && IterVersion->is_number_integer())
	{
		const int iVersion = IterVersion->get<int>();
		if (0 < iVersion)
			pOutDesc->Version = static_cast<_uint>(iVersion);
	}

	const auto IterPreset = jMapContent.find("PresetIndex");
	if (IterPreset != jMapContent.end() && IterPreset->is_number_integer())
		pOutDesc->iPresetIndex = IterPreset->get<int>();

	const auto IterManifest = jMapContent.find("Manifest");
	if (IterManifest != jMapContent.end() && IterManifest->is_string())
		pOutDesc->strManifestPath = StrToWstr(IterManifest->get<string>());

	const auto IterLoadStage = jMapContent.find("LoadStage");
	if (IterLoadStage != jMapContent.end() && IterLoadStage->is_boolean())
		pOutDesc->bLoadStage = IterLoadStage->get<bool>();

	const auto IterLoadEnv = jMapContent.find("LoadEnv");
	if (IterLoadEnv != jMapContent.end() && IterLoadEnv->is_boolean())
		pOutDesc->bLoadEnv = IterLoadEnv->get<bool>();

	const auto IterOverride = jMapContent.find("Override");
	if (IterOverride != jMapContent.end())
	{
		if (FAILED(Load_Change(*IterOverride, &pOutDesc->OverrideDesc)))
			return E_FAIL;
	}

	return S_OK;
}

json CMap_EditFile::Save_Change(const MAP_EDIT_CHANGE& Desc)
{
	json jOverride = json::object();
	jOverride["Version"] = Desc.Version;

	vector<_wstring> DeletedKeys(
		Desc.DeletedEnvObjectKeys.begin(),
		Desc.DeletedEnvObjectKeys.end());

	sort(DeletedKeys.begin(), DeletedKeys.end());

	json jDeletedEnvObjects = json::array();
	for (const auto& strKey : DeletedKeys)
		jDeletedEnvObjects.push_back(WstrToStr(strKey));

	jOverride["DeletedEnvObjects"] = jDeletedEnvObjects;

	jOverride["AddedMapObjects"] = json::array();
	for (const auto& Added : Desc.AddedMapObjects)
	{
		json jAdded = json::object();
		jAdded["Prototype_Tag"] = WstrToStr(Added.strPrototypeTag);
		jAdded["Layer_Tag"] = WstrToStr(Added.strLayerTag);
		jAdded["Object_Tag"] = WstrToStr(Added.strObjectTag);
		jAdded["Object"] = Added.jObject;
		jOverride["AddedMapObjects"].push_back(jAdded);
	}

	return jOverride;
}

HRESULT CMap_EditFile::Load_Change(const json& jOverride, MAP_EDIT_CHANGE* pOutDesc)
{
	if (nullptr == pOutDesc)
		return E_FAIL;

	*pOutDesc = {};

	if (!jOverride.is_object())
		return E_FAIL;

	const auto IterVersion = jOverride.find("Version");
	if (IterVersion != jOverride.end() && IterVersion->is_number_integer())
	{
		const int iVersion = IterVersion->get<int>();
		if (0 < iVersion)
			pOutDesc->Version = static_cast<_uint>(iVersion);
	}

	const auto IterDeleted = jOverride.find("DeletedEnvObjects");
	if (IterDeleted != jOverride.end())
	{
		if (!IterDeleted->is_array())
			return E_FAIL;

		for (const auto& jKey : *IterDeleted)
		{
			if (!jKey.is_string())
				continue;

			const _wstring strKey = StrToWstr(jKey.get<string>());
			if (!strKey.empty())
				pOutDesc->DeletedEnvObjectKeys.insert(strKey);
		}
	}

	const auto IterAdded = jOverride.find("AddedMapObjects");
	if (IterAdded != jOverride.end())
	{
		if (!IterAdded->is_array())
			return E_FAIL;

		for (const auto& jAdded : *IterAdded)
		{
			if (!jAdded.is_object())
				continue;

			MAP_ADD_OBJECT AddedDesc{};
			AddedDesc.jObject = json::object();

			const auto IterPrototype = jAdded.find("Prototype_Tag");
			if (IterPrototype != jAdded.end() && IterPrototype->is_string())
				AddedDesc.strPrototypeTag = StrToWstr(IterPrototype->get<string>());

			const auto IterLayer = jAdded.find("Layer_Tag");
			if (IterLayer != jAdded.end() && IterLayer->is_string())
				AddedDesc.strLayerTag = StrToWstr(IterLayer->get<string>());

			const auto IterObjectTag = jAdded.find("Object_Tag");
			if (IterObjectTag != jAdded.end() && IterObjectTag->is_string())
				AddedDesc.strObjectTag = StrToWstr(IterObjectTag->get<string>());

			const auto IterObject = jAdded.find("Object");
			if (IterObject != jAdded.end())
			{
				if (IterObject->is_object())
					AddedDesc.jObject = *IterObject;
				else if (!IterObject->is_null())
					continue;
			}

			if (AddedDesc.strPrototypeTag.empty()
				|| AddedDesc.strLayerTag.empty()
				|| AddedDesc.strObjectTag.empty())
			{
				continue;
			}

			pOutDesc->AddedMapObjects.push_back(AddedDesc);
		}
	}

	return S_OK;
}

json CMap_EditFile::Save_Stage(const CMapStage* pStage)
{
	if (nullptr == pStage)
		return json::object();

	json jOverride = json::object();
	jOverride["StageName"] = WstrToStr(pStage->Get_StageName());
	jOverride["Sections"] = json::array();

	const auto& Sections = pStage->Get_Sections();
	for (const CMapSection* pSection : Sections)
	{
		if (nullptr == pSection)
			continue;

		jOverride["Sections"].push_back(pSection->Serialize_SectionState());
	}

	return jOverride;
}

HRESULT CMap_EditFile::Apply_Stage(CMapStage* pStage, const json& jOverride)
{
	if (nullptr == pStage)
		return E_FAIL;

	if (jOverride.is_null())
		return S_OK;

	if (!jOverride.is_object())
		return E_FAIL;

	pStage->Deserialize(jOverride);
	return S_OK;
}

NS_END