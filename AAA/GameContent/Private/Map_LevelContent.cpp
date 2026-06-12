#include "Map_LevelContent.h"

NS_BEGIN(Client)

json CMap_LevelContent::Serialize(const MAP_LEVEL_CONTENT_DESC& Desc)
{
	json jMapContent = json::object();

	jMapContent["Version"] = Desc.Version;

	if (0 <= Desc.iPresetIndex)
		jMapContent["PresetIndex"] = Desc.iPresetIndex;

	if (!Desc.strManifestPath.empty())
		jMapContent["Manifest"] = WstrToStr(Desc.strManifestPath);

	jMapContent["LoadStage"] = static_cast<bool>(Desc.bLoadStage);
	jMapContent["LoadEnv"] = static_cast<bool>(Desc.bLoadEnv);
	jMapContent["Override"] = CMap_Override::Serialize(Desc.OverrideDesc);

	return jMapContent;
}

HRESULT CMap_LevelContent::Deserialize(const json& jLevelRoot, MAP_LEVEL_CONTENT_DESC* pOutDesc)
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
		if (FAILED(CMap_Override::Deserialize(*IterOverride, &pOutDesc->OverrideDesc)))
			return E_FAIL;
	}

	return S_OK;
}

NS_END