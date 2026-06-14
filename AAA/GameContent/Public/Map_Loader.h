#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"
#include "Map_LevelContent.h"
#include "Map_Override.h"

NS_BEGIN(Client)

class CMapStage;

struct MAP_RUNTIME_LOAD_CONTEXT
{
	ID3D11Device* pDevice = { nullptr };
	ID3D11DeviceContext* pContext = { nullptr };

	_uint iPlaceLevel = {};
	_uint iModelLevel = ETOUI(LEVEL::STATIC);

	MAP_OBJECT_CREATED_CALLBACK pCreatedCallback = { nullptr };
	void* pCallbackContext = { nullptr };
};

class CLIENT_DLL CMap_Loader final : public CBase
{
public:
	CMap_Loader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMap_Loader() = default;

public:
	HRESULT Initialize();

	HRESULT Load_FromManifest(
		const _wstring& strManifestPath,
		const MAP_RUNTIME_LEVELS& Levels,
		const MAP_SPAWN_TARGETS& Targets,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	HRESULT Build_Package(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage);
	HRESULT Ready_Prototypes(const MAP_RUNTIME_LEVELS& Levels, const MAP_PACKAGE& Package);
	HRESULT Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_REPORT* pOutReport = nullptr);

	static HRESULT Preload_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strManifestPath,
		_uint iRuntimeLevel);

	static HRESULT Preload_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel);

	static HRESULT Spawn_Map(
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	static HRESULT Spawn_Map(
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	static HRESULT Spawn_Map_WithOverride(
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		const MAP_OVERRIDE_DESC* pOverrideDesc,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	static HRESULT Preload_MapForLevel(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel);

	static HRESULT Spawn_MapForLevel(
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	static HRESULT Load_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	static HRESULT Load_MapStage_Runtime(
		const MAP_RUNTIME_LOAD_CONTEXT& Context,
		const _wstring& strMapManifestPath,
		CMapStage** ppOutStage = nullptr,
		json* pOutMapStageOverride = nullptr,
		_bool* pOutHasMapStageOverride = nullptr);

	static HRESULT Load_Env_Runtime(
		const MAP_RUNTIME_LOAD_CONTEXT& Context,
		const _wstring& strMapManifestPath,
		const MAP_OVERRIDE_DESC* pOverrideDesc = nullptr,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs = nullptr,
		_bool bEnableEnvObjectPicking = false);

	static _uint		Get_MapPresetCount();
	static const _char*	Get_MapPresetLabel(_uint iPresetIndex);
	static HRESULT		Get_MapPresetManifestPath(_uint iPresetIndex, _wstring* pOutManifestPath);
	static _bool		Is_MapLayer(const _wstring& strLayerTag);

	static HRESULT		Get_MapOverrideAssetPath(const _wstring& strManifestPath, _wstring* pOutOverridePath);
	static HRESULT		Load_MapOverrideAsset(const _wstring& strManifestPath, MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc, json* pOutMapStageOverride = nullptr, _bool* pOutHasMapStageOverride = nullptr);
	static HRESULT		Save_MapOverrideAsset(const MAP_LEVEL_CONTENT_DESC& MapContentDesc, const CMapStage* pStage);

	static HRESULT		Get_MapPresetOverrideAssetPath(_uint iPresetIndex, const _wstring& strManifestPath, _wstring* pOutOverridePath);
	static HRESULT		Load_MapPresetOverrideAsset(_uint iPresetIndex, const _wstring& strManifestPath, MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc, json* pOutMapStageOverride = nullptr, _bool* pOutHasMapStageOverride = nullptr);
	static HRESULT		Save_MapPresetOverrideAsset(_uint iPresetIndex, const MAP_LEVEL_CONTENT_DESC& MapContentDesc, const CMapStage* pStage);

	static json			Serialize_MapStageOverride(const CMapStage* pStage);
	static HRESULT		Apply_MapStageOverride(CMapStage* pStage, const json& jOverride);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pProxy = { nullptr };

private:
	static void Build_DefaultRuntimeLevels(_uint iRuntimeLevel, MAP_RUNTIME_LEVELS* pOutLevels);
	static void Build_DefaultRuntimeTargets(_uint iRuntimeLevel, MAP_SPAWN_TARGETS* pOutTargets);

public:
	static CMap_Loader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual void Free() override;
};

NS_END