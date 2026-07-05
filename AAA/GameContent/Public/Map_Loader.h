#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"
#include "Map_EditFile.h"

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
	CEnv_InstanceController** ppOutEnvInstanceController = { nullptr };
};

class CLIENT_DLL CMap_Loader final : public CBase
{
public:
	CMap_Loader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMap_Loader() = default;

public:
	HRESULT Initialize();

	static HRESULT Preload_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		const MAP_LOAD_OPTIONS& Options = MAP_LOAD_OPTIONS{});

	static HRESULT Preload_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel,
		const MAP_LOAD_OPTIONS& Options = MAP_LOAD_OPTIONS{});

	static HRESULT Spawn_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		MAP_LOAD_RESULT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr,
		const MAP_LOAD_OPTIONS& Options = MAP_LOAD_OPTIONS{},
		const MAP_EDIT_CHANGE* pLevelDesignOverrideDesc = nullptr);

	static HRESULT Spawn_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel,
		MAP_LOAD_RESULT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr,
		const MAP_LOAD_OPTIONS& Options = MAP_LOAD_OPTIONS{});

	static HRESULT Load_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		MAP_LOAD_RESULT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr,
		const MAP_LOAD_OPTIONS& Options = MAP_LOAD_OPTIONS{});

	static HRESULT Load_MapStage_Runtime(
		const MAP_RUNTIME_LOAD_CONTEXT& Context,
		const _wstring& strMapManifestPath,
		CMapStage** ppOutStage = nullptr,
		json* pOutMapStageOverride = nullptr,
		_bool* pOutHasMapStageOverride = nullptr);

	static HRESULT Load_Env_Runtime(
		const MAP_RUNTIME_LOAD_CONTEXT& Context,
		const _wstring& strMapManifestPath,
		const MAP_EDIT_CHANGE* pOverrideDesc = nullptr,
		MAP_LOAD_RESULT* pOutReport = nullptr,
		vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs = nullptr,
		_bool bEnableEnvObjectPicking = false);

	static HRESULT Load_LevelDesign_Runtime(
		const MAP_RUNTIME_LOAD_CONTEXT& Context,
		const _wstring& strMapManifestPath,
		MAP_LOAD_RESULT* pOutReport = nullptr,
		const MAP_EDIT_CHANGE* pOverrideDesc = nullptr);

	static _uint			Get_MapCount();
	static const _char*		Get_MapName(_uint iMapIndex);
	static HRESULT			Get_MapManifestPath(_uint iMapIndex, _wstring* pOutManifestPath);
	
	static _bool			Is_MapLayer(const _wstring& strLayerTag);
	static _bool			Is_MapStageLayer(const _wstring& strLayerTag);
	static _bool			Is_MapEnvLayer(const _wstring& strLayerTag);

	static HRESULT			Get_EditFilePath(const _wstring& strManifestPath, _wstring* pOutEditFilePath);
	static HRESULT			Load_EditFile(const _wstring& strManifestPath, MAP_EDIT_DATA* pInOutData, json* pOutStageEdit = nullptr, _bool* pOutHasStageEdit = nullptr);
	static HRESULT			Save_EditFile(const MAP_EDIT_DATA& Data, const CMapStage* pStage);

	static HRESULT			Get_PresetEditFilePath(_uint iMapIndex, const _wstring& strManifestPath, _wstring* pOutEditFilePath);
	static HRESULT			Load_PresetEditFile(_uint iMapIndex, const _wstring& strManifestPath, MAP_EDIT_DATA* pInOutData, json* pOutStageEdit = nullptr, _bool* pOutHasStageEdit = nullptr);
	static HRESULT			Save_PresetEditFile(_uint iMapIndex, const MAP_EDIT_DATA& Data, const CMapStage* pStage);

	static HRESULT			Ready_TexHub(CGameInstance_Proxy* pProxy);

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance_Proxy*	m_pProxy = { nullptr };

private:
	HRESULT Load_FromManifest(
		const _wstring& strManifestPath,
		const MAP_RUNTIME_LEVELS& Levels,
		const MAP_SPAWN_TARGETS& Targets,
		const MAP_LOAD_OPTIONS& Options,
		MAP_LOAD_RESULT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	HRESULT Preload_LevelDesignEntries(
		const MAP_PACKAGE& Package,
		const MAP_RUNTIME_LEVELS& Levels);

	HRESULT Load_LevelDesignEntries(
		const MAP_PACKAGE& Package,
		const MAP_SPAWN_REQUEST& Request,
		MAP_LOAD_RESULT* pOutReport = nullptr,
		const MAP_EDIT_CHANGE* pOverrideDesc = nullptr);

	HRESULT Build_Package(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage);
	HRESULT Build_Package(const _wstring& strManifestPath, const MAP_PACKAGE_BUILD_OPTIONS& BuildOptions, MAP_PACKAGE* pOutPackage);
	HRESULT Ready_Prototypes(const MAP_RUNTIME_LEVELS& Levels, const MAP_PACKAGE& Package);
	HRESULT Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_RESULT* pOutReport = nullptr);

	static void Build_DefaultRuntimeLevels(_uint iRuntimeLevel, MAP_RUNTIME_LEVELS* pOutLevels);
	static void Build_DefaultRuntimeTargets(_uint iRuntimeLevel, MAP_SPAWN_TARGETS* pOutTargets);

public:
	static CMap_Loader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual void Free() override;
};

NS_END
