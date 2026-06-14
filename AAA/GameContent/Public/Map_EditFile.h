#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Client)

class CMapStage;

class CLIENT_DLL CMap_EditFile final
{
public:
	static _wstring Make_EnvKey(const ENV_OBJECT_DESC& Desc);

	static HRESULT Apply_Change(
		MAP_PACKAGE* pInOutPackage,
		const MAP_EDIT_CHANGE& OverrideDesc);

public:
	static HRESULT Get_EditFilePath(
		const _wstring& strManifestPath,
		_wstring* pOutEditFilePath);

	static HRESULT Get_PresetEditFilePath(
		_uint iPresetIndex,
		const _wstring& strManifestPath,
		_wstring* pOutEditFilePath);

	static HRESULT Load_EditFile(
		const _wstring& strManifestPath,
		MAP_EDIT_DATA* pInOutMapContentDesc,
		json* pOutMapStageOverride = nullptr,
		_bool* pOutHasMapStageOverride = nullptr);

	static HRESULT Load_PresetEditFile(
		_uint iPresetIndex,
		const _wstring& strManifestPath,
		MAP_EDIT_DATA* pInOutMapContentDesc,
		json* pOutMapStageOverride = nullptr,
		_bool* pOutHasMapStageOverride = nullptr);

	static HRESULT Save_EditFile(
		const MAP_EDIT_DATA& MapContentDesc,
		const CMapStage* pStage);

	static HRESULT Save_PresetEditFile(
		_uint iPresetIndex,
		const MAP_EDIT_DATA& MapContentDesc,
		const CMapStage* pStage);

public:
	static json Save_Data(const MAP_EDIT_DATA& Desc);
	static HRESULT Load_Data(const json& jLevelRoot, MAP_EDIT_DATA* pOutDesc);

	static json Save_Change(const MAP_EDIT_CHANGE& Desc);
	static HRESULT Load_Change(const json& jOverride, MAP_EDIT_CHANGE* pOutDesc);

	static json Save_Stage(const CMapStage* pStage);
	static HRESULT Apply_Stage(CMapStage* pStage, const json& jOverride);
};

NS_END