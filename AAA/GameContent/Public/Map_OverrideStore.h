#pragma once
#include "Map_LevelContent.h"

NS_BEGIN(Client)
class CMapStage;

class CLIENT_DLL CMap_OverrideStore final
{
public:
	static HRESULT Get_OverrideAssetPath(const _wstring& strManifestPath, _wstring* pOutOverridePath);

	static HRESULT Load_OverrideAsset(const _wstring& strManifestPath, MAP_EDIT_DATA* pInOutMapContentDesc,
		json* pOutMapStageOverride = nullptr, _bool* pOutHasMapStageOverride = nullptr);

	static HRESULT Save_OverrideAsset(const MAP_EDIT_DATA& MapContentDesc, const CMapStage* pStage);

	static HRESULT Get_PresetOverrideAssetPath(_uint iPresetIndex, const _wstring& strManifestPath, _wstring* pOutOverridePath);

	static HRESULT Load_PresetOverrideAsset(_uint iPresetIndex, const _wstring& strManifestPath, MAP_EDIT_DATA* pInOutMapContentDesc,
		json* pOutMapStageOverride = nullptr, _bool* pOutHasMapStageOverride = nullptr);

	static HRESULT Save_PresetOverrideAsset(_uint iPresetIndex, const MAP_EDIT_DATA& MapContentDesc, const CMapStage* pStage);
};

NS_END