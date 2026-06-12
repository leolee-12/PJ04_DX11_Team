#include "Map_Loader.h"
#include "Map_LayerPolicy.h"
#include "Map_PresetCatalog.h"
#include "Map_LevelContent.h"
#include "Map_OverrideStore.h"
#include "Map_StageOverrideSerializer.h"
#include "Map_Builder.h"
#include "Map_ModelResolver.h"
#include "Map_ProtoRegister.h"
#include "Map_Spawner.h"

#include "GameInstance.h"
#include "DataLoader.h"

#include <mutex>

namespace
{
	std::mutex g_MapPackageCacheMutex;
	std::unordered_map<std::wstring, MAP_PACKAGE> g_MapPackageCache;

	std::wstring Make_MapCacheKey(const std::wstring& strManifestPath, _uint iRuntimeLevel)
	{
		return std::to_wstring(iRuntimeLevel) + L"|" + strManifestPath;
	}

	void Store_MapPackage(const std::wstring& strManifestPath, _uint iRuntimeLevel, const MAP_PACKAGE& Package)
	{
		std::lock_guard<std::mutex> lock(g_MapPackageCacheMutex);
		g_MapPackageCache[Make_MapCacheKey(strManifestPath, iRuntimeLevel)] = Package;
	}

	bool Try_GetMapPackage(const std::wstring& strManifestPath,
		_uint iRuntimeLevel,
		MAP_PACKAGE* pOutPackage)
	{
		if (nullptr == pOutPackage)
			return false;

		std::lock_guard<std::mutex> lock(g_MapPackageCacheMutex);

		const auto iter = g_MapPackageCache.find(Make_MapCacheKey(strManifestPath, iRuntimeLevel));
		if (iter == g_MapPackageCache.end())
			return false;

		*pOutPackage = iter->second;
		return true;
	}

	bool Try_LoadLevelMapContent(
		const std::wstring& strLevelObjectsPath,
		Client::MAP_LEVEL_CONTENT_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return false;

		*pOutDesc = {};

		if (strLevelObjectsPath.empty())
			return false;

		std::string strContent{};
		if (FAILED(CDataLoader::Read_Json(strLevelObjectsPath.c_str(), &strContent)))
			return false;

		try
		{
			json jLevel = json::parse(strContent);
			return SUCCEEDED(Client::CMap_LevelContent::Deserialize(jLevel, pOutDesc));
		}
		catch (const json::exception&)
		{
			return false;
		}
	}

	bool Is_RuntimeLoadContextValid(const Client::MAP_RUNTIME_LOAD_CONTEXT& Context)
	{
		return nullptr != Context.pDevice
			&& nullptr != Context.pContext;
	}

	void Collect_DeletedEnvDescs(
		const vector<Client::ENV_OBJECT_DESC>& SourceDescs,
		const Client::MAP_OVERRIDE_DESC* pOverrideDesc,
		vector<Client::ENV_OBJECT_DESC>* pOutDeletedEnvDescs)
	{
		if (nullptr == pOutDeletedEnvDescs)
			return;

		pOutDeletedEnvDescs->clear();

		if (nullptr == pOverrideDesc)
			return;

		for (const auto& Desc : SourceDescs)
		{
			const _wstring strKey = Client::CMap_Override::Build_EnvObjectStableKey(Desc);
			if (pOverrideDesc->DeletedEnvObjectKeys.find(strKey) != pOverrideDesc->DeletedEnvObjectKeys.end())
				pOutDeletedEnvDescs->push_back(Desc);
		}
	}

	void Build_RuntimeStageLevels(
		const Client::MAP_RUNTIME_LOAD_CONTEXT& Context,
		Client::MAP_RUNTIME_LEVELS* pOutLevels)
	{
		if (nullptr == pOutLevels)
			return;

		*pOutLevels = {};
		pOutLevels->iObjectLevel = ETOUI(Client::LEVEL::STATIC);
		pOutLevels->iStageModelLevel = Context.iModelLevel;
		pOutLevels->iEnvModelLevel = ETOUI(Client::LEVEL::STATIC);
	}

	void Build_RuntimeEnvLevels(
		const Client::MAP_RUNTIME_LOAD_CONTEXT& Context,
		_bool bEnableEnvObjectPicking,
		Client::MAP_RUNTIME_LEVELS* pOutLevels)
	{
		if (nullptr == pOutLevels)
			return;

		*pOutLevels = {};
		pOutLevels->iObjectLevel = ETOUI(Client::LEVEL::STATIC);
		pOutLevels->iStageModelLevel = ETOUI(Client::LEVEL::STATIC);
		pOutLevels->iEnvModelLevel = Context.iModelLevel;
		pOutLevels->bEnableEnvObjectPicking = bEnableEnvObjectPicking;
	}

	HRESULT Resolve_LevelMapRequest(
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_wstring* pOutManifestPath,
		MAP_LEVEL_CONTENT_DESC* pOutMapContentDesc,
		json* pOutMapStageOverride = nullptr,
		_bool* pOutHasMapStageOverride = nullptr)
	{
		if (nullptr == pOutManifestPath || nullptr == pOutMapContentDesc)
			return E_FAIL;

		*pOutManifestPath = strFallbackManifestPath;
		*pOutMapContentDesc = {};

		if (nullptr != pOutMapStageOverride)
			*pOutMapStageOverride = json::object();

		if (nullptr != pOutHasMapStageOverride)
			*pOutHasMapStageOverride = false;

		Try_LoadLevelMapContent(strLevelObjectsPath, pOutMapContentDesc);

		if (pOutMapContentDesc->bHasMapContent
			&& !pOutMapContentDesc->strManifestPath.empty())
		{
			*pOutManifestPath = pOutMapContentDesc->strManifestPath;
		}

		if (pOutManifestPath->empty())
			return E_FAIL;

		if (pOutMapContentDesc->strManifestPath.empty())
			pOutMapContentDesc->strManifestPath = *pOutManifestPath;

		const HRESULT hrAsset = Client::CMap_OverrideStore::Load_OverrideAsset(
			*pOutManifestPath,
			pOutMapContentDesc,
			pOutMapStageOverride,
			pOutHasMapStageOverride);

		if (S_FALSE == hrAsset)
			return S_OK;

		return hrAsset;
	}
}

NS_BEGIN(Client)

CMap_Loader::CMap_Loader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pProxy{ CGameInstance::GetProxy() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CMap_Loader::Initialize()
{
	return nullptr != m_pProxy ? S_OK : E_FAIL;
}

HRESULT CMap_Loader::Load_FromManifest(const _wstring& strManifestPath, const MAP_RUNTIME_LEVELS& Levels,
	const MAP_SPAWN_TARGETS& Targets, MAP_LOAD_REPORT* pOutReport, CMapStage** ppOutStage)
{
	MAP_PACKAGE Package{};

	HRESULT hr = Build_Package(strManifestPath, &Package);
	if (FAILED(hr))
		return hr;

	hr = Ready_Prototypes(Levels, Package);
	if (FAILED(hr))
		return hr;

	MAP_SPAWN_REQUEST Request{};
	Request.Levels = Levels;
	Request.Targets = Targets;
	Request.Options.bSpawnStage = true;
	Request.Options.bSpawnEnv = true;
	Request.ppOutStage = ppOutStage;

	return Spawn(Package, Request, pOutReport);
}

HRESULT CMap_Loader::Build_Package(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage)
{
	if (nullptr == pOutPackage)
		return E_FAIL;

	CMap_ModelResolver* pResolver = CMap_ModelResolver::Create();
	if (nullptr == pResolver)
		return E_FAIL;

	CMap_Builder* pBuilder = CMap_Builder::Create(pResolver);
	if (nullptr == pBuilder)
	{
		Safe_Release(pResolver);
		return E_FAIL;
	}

	const HRESULT hr = pBuilder->Build_FromManifest(strManifestPath, pOutPackage);

	Safe_Release(pBuilder);
	Safe_Release(pResolver);

	return hr;
}

HRESULT CMap_Loader::Ready_Prototypes(const MAP_RUNTIME_LEVELS& Levels, const MAP_PACKAGE& Package)
{
	if (nullptr == m_pDevice || nullptr == m_pContext)
		return E_FAIL;

	CMap_ProtoRegister* pRegister = CMap_ProtoRegister::Create(m_pDevice, m_pContext);
	if (nullptr == pRegister)
		return E_FAIL;

	const HRESULT hr = pRegister->Ready_Prototypes(Levels, Package);

	Safe_Release(pRegister);

	return hr;
}

HRESULT CMap_Loader::Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_REPORT* pOutReport)
{
	CMap_Spawner* pSpawner = CMap_Spawner::Create();
	if (nullptr == pSpawner)
		return E_FAIL;

	const HRESULT hr = pSpawner->Spawn(Package, Request, pOutReport);

	Safe_Release(pSpawner);

	return hr;
}

HRESULT CMap_Loader::Preload_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	const _wstring& strManifestPath, _uint iRuntimeLevel)
{
	if (nullptr == pDevice || nullptr == pContext || strManifestPath.empty())
		return E_FAIL;

	CMap_Loader* pMapLoader = Create(pDevice, pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strManifestPath, &Package);

	if (SUCCEEDED(hr))
	{
		MAP_RUNTIME_LEVELS Levels{};
		Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);

		hr = pMapLoader->Ready_Prototypes(Levels, Package);
	}

	if (SUCCEEDED(hr))
		Store_MapPackage(strManifestPath, iRuntimeLevel, Package);

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Preload_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	const _wstring& strFallbackManifestPath, const _wstring& strLevelObjectsPath, _uint iRuntimeLevel)
{
	if (nullptr == pDevice || nullptr == pContext)
		return E_FAIL;

	_wstring strResolvedManifestPath;
	MAP_LEVEL_CONTENT_DESC MapContentDesc{};
	if (FAILED(Resolve_LevelMapRequest(strFallbackManifestPath, strLevelObjectsPath, &strResolvedManifestPath, &MapContentDesc)))
	{
		return E_FAIL;
	}

	CMap_Loader* pMapLoader = Create(pDevice, pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strResolvedManifestPath, &Package);

	if (SUCCEEDED(hr) && MapContentDesc.bHasMapContent)
		hr = CMap_Override::Apply(&Package, MapContentDesc.OverrideDesc);

	if (SUCCEEDED(hr))
	{
		MAP_RUNTIME_LEVELS Levels{};
		Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);

		hr = pMapLoader->Ready_Prototypes(Levels, Package);
	}

	if (SUCCEEDED(hr))
		Store_MapPackage(strResolvedManifestPath, iRuntimeLevel, Package);

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Preload_MapForLevel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
const _wstring& strFallbackManifestPath, const _wstring& strLevelObjectsPath, _uint iRuntimeLevel)
{
	return Preload_Map(pDevice, pContext, strFallbackManifestPath, strLevelObjectsPath, iRuntimeLevel);
}

HRESULT CMap_Loader::Spawn_Map(const _wstring& strManifestPath, _uint iRuntimeLevel,
	MAP_LOAD_REPORT* pOutReport, CMapStage** ppOutStage)
{
	return Spawn_Map_WithOverride(strManifestPath, iRuntimeLevel, nullptr, pOutReport, ppOutStage);
}

HRESULT CMap_Loader::Spawn_Map_WithOverride(const _wstring& strManifestPath, _uint iRuntimeLevel,
	const MAP_OVERRIDE_DESC* pOverrideDesc, MAP_LOAD_REPORT* pOutReport, CMapStage** ppOutStage)
{
	if (strManifestPath.empty())
		return E_FAIL;

	MAP_PACKAGE Package{};
	if (!Try_GetMapPackage(strManifestPath, iRuntimeLevel, &Package))
		return E_FAIL;

	if (pOverrideDesc != nullptr)
	{
		const HRESULT hrApply = CMap_Override::Apply(&Package, *pOverrideDesc);
		if (FAILED(hrApply))
			return hrApply;
	}

	MAP_RUNTIME_LEVELS Levels{};
	MAP_SPAWN_TARGETS Targets{};
	Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);
	Build_DefaultRuntimeTargets(iRuntimeLevel, &Targets);

	MAP_SPAWN_REQUEST Request{};
	Request.Levels = Levels;
	Request.Targets = Targets;
	Request.Options.bSpawnStage = true;
	Request.Options.bSpawnEnv = true;
	Request.ppOutStage = ppOutStage;

	CMap_Spawner* pSpawner = CMap_Spawner::Create();
	if (nullptr == pSpawner)
		return E_FAIL;

	const HRESULT hr = pSpawner->Spawn(Package, Request, pOutReport);

	Safe_Release(pSpawner);
	return hr;
}

HRESULT CMap_Loader::Spawn_Map(
	const _wstring& strFallbackManifestPath,
	const _wstring& strLevelObjectsPath,
	_uint iRuntimeLevel,
	MAP_LOAD_REPORT* pOutReport,
	CMapStage** ppOutStage)
{
	_wstring strResolvedManifestPath;
	MAP_LEVEL_CONTENT_DESC MapContentDesc{};
	json jMapStageOverride = json::object();
	_bool bHasMapStageOverride = false;

	if (FAILED(Resolve_LevelMapRequest(
		strFallbackManifestPath,
		strLevelObjectsPath,
		&strResolvedManifestPath,
		&MapContentDesc,
		&jMapStageOverride,
		&bHasMapStageOverride)))
	{
		return E_FAIL;
	}

	CMapStage* pLocalStage = nullptr;
	CMapStage** ppStageForSpawn = ppOutStage;

	if (nullptr == ppStageForSpawn && bHasMapStageOverride)
		ppStageForSpawn = &pLocalStage;

	const HRESULT hrSpawn = Spawn_Map(
		strResolvedManifestPath,
		iRuntimeLevel,
		pOutReport,
		ppStageForSpawn);

	if (FAILED(hrSpawn))
		return hrSpawn;

	if (bHasMapStageOverride)
	{
		CMapStage* pStageToApply = nullptr != ppOutStage ? *ppOutStage : pLocalStage;
		if (FAILED(CMap_StageOverrideSerializer::Apply(pStageToApply, jMapStageOverride)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMap_Loader::Spawn_MapForLevel(
	const _wstring& strFallbackManifestPath,
	const _wstring& strLevelObjectsPath,
	_uint iRuntimeLevel,
	MAP_LOAD_REPORT* pOutReport,
	CMapStage** ppOutStage)
{
	return Spawn_Map(
		strFallbackManifestPath,
		strLevelObjectsPath,
		iRuntimeLevel,
		pOutReport,
		ppOutStage);
}

HRESULT CMap_Loader::Load_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	const _wstring& strManifestPath, _uint iRuntimeLevel, MAP_LOAD_REPORT* pOutReport, CMapStage** ppOutStage)
{
	if (nullptr == pDevice || nullptr == pContext || strManifestPath.empty())
		return E_FAIL;

	CMap_Loader* pMapLoader = Create(pDevice, pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_RUNTIME_LEVELS Levels{};
	MAP_SPAWN_TARGETS Targets{};
	Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);
	Build_DefaultRuntimeTargets(iRuntimeLevel, &Targets);

	const HRESULT hr = pMapLoader->Load_FromManifest(
		strManifestPath,
		Levels,
		Targets,
		pOutReport,
		ppOutStage);

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Load_MapStage_Runtime(
	const MAP_RUNTIME_LOAD_CONTEXT& Context,
	const _wstring& strMapManifestPath,
	CMapStage** ppOutStage,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	if (nullptr != ppOutStage)
		*ppOutStage = nullptr;

	if (nullptr != pOutMapStageOverride)
		*pOutMapStageOverride = json::object();

	if (nullptr != pOutHasMapStageOverride)
		*pOutHasMapStageOverride = false;

	if (!Is_RuntimeLoadContextValid(Context) || strMapManifestPath.empty())
		return E_FAIL;

	MAP_LEVEL_CONTENT_DESC MapContentDesc{};
	json jLocalMapStageOverride = json::object();
	_bool bLocalHasMapStageOverride = false;

	json* pStageOverride = nullptr != pOutMapStageOverride ? pOutMapStageOverride : &jLocalMapStageOverride;
	_bool* pHasStageOverride = nullptr != pOutHasMapStageOverride ? pOutHasMapStageOverride : &bLocalHasMapStageOverride;

	const HRESULT hrAsset = CMap_OverrideStore::Load_OverrideAsset(
		strMapManifestPath,
		&MapContentDesc,
		pStageOverride,
		pHasStageOverride);

	if (FAILED(hrAsset) && S_FALSE != hrAsset)
		return hrAsset;

	CMap_Loader* pMapLoader = Create(Context.pDevice, Context.pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strMapManifestPath, &Package);

	if (SUCCEEDED(hr))
	{
		Package.EnvObjectDescs.clear();
		Package.EnvJsonPaths.clear();
		Package.AddedObjectDescs.clear();

		MAP_RUNTIME_LEVELS Levels{};
		Build_RuntimeStageLevels(Context, &Levels);

		hr = pMapLoader->Ready_Prototypes(Levels, Package);
		if (SUCCEEDED(hr))
		{
			CMapStage* pLocalStage = nullptr;
			CMapStage** ppStageForSpawn = nullptr != ppOutStage ? ppOutStage : &pLocalStage;

			MAP_SPAWN_REQUEST Request{};
			Request.Levels = Levels;
			Build_DefaultRuntimeTargets(Context.iPlaceLevel, &Request.Targets);
			Request.Options.bSpawnStage = true;
			Request.Options.bSpawnEnv = false;
			Request.pCreatedCallback = Context.pCreatedCallback;
			Request.pCallbackContext = Context.pCallbackContext;
			Request.ppOutStage = ppStageForSpawn;

			hr = pMapLoader->Spawn(Package, Request, nullptr);

			if (SUCCEEDED(hr) && *pHasStageOverride)
			{
				CMapStage* pStageToApply = nullptr != ppOutStage ? *ppOutStage : pLocalStage;
				hr = CMap_StageOverrideSerializer::Apply(pStageToApply, *pStageOverride);
			}
		}
	}

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Load_Env_Runtime(
	const MAP_RUNTIME_LOAD_CONTEXT& Context,
	const _wstring& strMapManifestPath,
	const MAP_OVERRIDE_DESC* pOverrideDesc,
	MAP_LOAD_REPORT* pOutReport,
	vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs,
	_bool bEnableEnvObjectPicking)
{
	if (nullptr != pOutReport)
		*pOutReport = {};

	if (nullptr != pOutDeletedEnvDescs)
		pOutDeletedEnvDescs->clear();

	if (!Is_RuntimeLoadContextValid(Context) || strMapManifestPath.empty())
		return E_FAIL;

	MAP_LEVEL_CONTENT_DESC LoadedMapContentDesc{};
	const MAP_OVERRIDE_DESC* pResolvedOverrideDesc = pOverrideDesc;

	if (nullptr == pResolvedOverrideDesc)
	{
		const HRESULT hrAsset = CMap_OverrideStore::Load_OverrideAsset(
			strMapManifestPath,
			&LoadedMapContentDesc);

		if (FAILED(hrAsset) && S_FALSE != hrAsset)
			return hrAsset;

		if (LoadedMapContentDesc.bHasMapContent)
			pResolvedOverrideDesc = &LoadedMapContentDesc.OverrideDesc;
	}

	CMap_Loader* pMapLoader = Create(Context.pDevice, Context.pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_PACKAGE SourcePackage{};
	HRESULT hr = pMapLoader->Build_Package(strMapManifestPath, &SourcePackage);

	if (SUCCEEDED(hr))
	{
		Collect_DeletedEnvDescs(
			SourcePackage.EnvObjectDescs,
			pResolvedOverrideDesc,
			pOutDeletedEnvDescs);

		MAP_PACKAGE SpawnPackage = SourcePackage;
		SpawnPackage.StageDesc = {};

		if (nullptr != pResolvedOverrideDesc)
			hr = CMap_Override::Apply(&SpawnPackage, *pResolvedOverrideDesc);

		if (SUCCEEDED(hr))
		{
			MAP_RUNTIME_LEVELS Levels{};
			Build_RuntimeEnvLevels(Context, bEnableEnvObjectPicking, &Levels);

			hr = pMapLoader->Ready_Prototypes(Levels, SpawnPackage);
			if (SUCCEEDED(hr))
			{
				MAP_SPAWN_REQUEST Request{};
				Request.Levels = Levels;
				Build_DefaultRuntimeTargets(Context.iPlaceLevel, &Request.Targets);
				Request.Options.bSpawnStage = false;
				Request.Options.bSpawnEnv = true;
				Request.pCreatedCallback = Context.pCreatedCallback;
				Request.pCallbackContext = Context.pCallbackContext;

				hr = pMapLoader->Spawn(SpawnPackage, Request, pOutReport);
			}
		}
	}

	Safe_Release(pMapLoader);
	return hr;
}

_uint CMap_Loader::Get_MapPresetCount()
{
	return CMap_PresetCatalog::Get_Count();
}

const _char* CMap_Loader::Get_MapPresetLabel(_uint iPresetIndex)
{
	return CMap_PresetCatalog::Get_Label(iPresetIndex);
}

HRESULT CMap_Loader::Get_MapPresetManifestPath(_uint iPresetIndex, _wstring* pOutManifestPath)
{
	return CMap_PresetCatalog::Get_ManifestPath(iPresetIndex, pOutManifestPath);
}

_bool CMap_Loader::Is_MapLayer(const _wstring& strLayerTag)
{
	return CMap_LayerPolicy::Is_MapLayer(strLayerTag);
}

HRESULT CMap_Loader::Get_MapOverrideAssetPath(const _wstring& strManifestPath, _wstring* pOutOverridePath)
{
	return CMap_OverrideStore::Get_OverrideAssetPath(strManifestPath, pOutOverridePath);
}

HRESULT CMap_Loader::Load_MapOverrideAsset(const _wstring& strManifestPath, MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc, json* pOutMapStageOverride, _bool* pOutHasMapStageOverride)
{
	return CMap_OverrideStore::Load_OverrideAsset(strManifestPath, pInOutMapContentDesc, pOutMapStageOverride, pOutHasMapStageOverride);
}

HRESULT CMap_Loader::Save_MapOverrideAsset(const MAP_LEVEL_CONTENT_DESC& MapContentDesc, const CMapStage* pStage)
{
	return CMap_OverrideStore::Save_OverrideAsset(MapContentDesc, pStage);
}

HRESULT CMap_Loader::Get_MapPresetOverrideAssetPath(_uint iPresetIndex, const _wstring& strManifestPath, _wstring* pOutOverridePath)
{
	return CMap_OverrideStore::Get_PresetOverrideAssetPath(iPresetIndex, strManifestPath, pOutOverridePath);
}

HRESULT CMap_Loader::Load_MapPresetOverrideAsset(_uint iPresetIndex, const _wstring& strManifestPath, MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc, json* pOutMapStageOverride, _bool* pOutHasMapStageOverride)
{
	return CMap_OverrideStore::Load_PresetOverrideAsset(iPresetIndex, strManifestPath, pInOutMapContentDesc, pOutMapStageOverride, pOutHasMapStageOverride);
}

HRESULT CMap_Loader::Save_MapPresetOverrideAsset(_uint iPresetIndex, const MAP_LEVEL_CONTENT_DESC& MapContentDesc, const CMapStage* pStage)
{
	return CMap_OverrideStore::Save_PresetOverrideAsset(iPresetIndex, MapContentDesc, pStage);
}

json CMap_Loader::Serialize_MapStageOverride(const CMapStage* pStage)
{
	return CMap_StageOverrideSerializer::Serialize(pStage);
}

HRESULT CMap_Loader::Apply_MapStageOverride(CMapStage* pStage, const json& jOverride)
{
	return CMap_StageOverrideSerializer::Apply(pStage, jOverride);
}

void CMap_Loader::Build_DefaultRuntimeLevels(_uint iRuntimeLevel, MAP_RUNTIME_LEVELS* pOutLevels)
{
	if (nullptr == pOutLevels)
		return;

	*pOutLevels = {};
	//pOutLevels->iObjectLevel = iRuntimeLevel;
	//pOutLevels->iStageModelLevel = iRuntimeLevel;
	//pOutLevels->iEnvModelLevel = iRuntimeLevel;
	pOutLevels->iObjectLevel = ETOUI(LEVEL::STATIC);
	pOutLevels->iStageModelLevel = ETOUI(LEVEL::STATIC);
	pOutLevels->iEnvModelLevel = ETOUI(LEVEL::STATIC);
}

void CMap_Loader::Build_DefaultRuntimeTargets(_uint iRuntimeLevel, MAP_SPAWN_TARGETS* pOutTargets)
{
	if (nullptr == pOutTargets)
		return;

	*pOutTargets = {};

	pOutTargets->Stage.iPlaceLevel = iRuntimeLevel;
	pOutTargets->Stage.pLayerTag = CMap_LayerPolicy::LAYER_MAP_STAGE;

	pOutTargets->EnvStatic.iPlaceLevel = iRuntimeLevel;
	pOutTargets->EnvStatic.pLayerTag = CMap_LayerPolicy::LAYER_ENV_STATIC;

	pOutTargets->EnvInteract.iPlaceLevel = iRuntimeLevel;
	pOutTargets->EnvInteract.pLayerTag = CMap_LayerPolicy::LAYER_ENV_INTERACT;

	pOutTargets->EnvEffect.iPlaceLevel = iRuntimeLevel;
	pOutTargets->EnvEffect.pLayerTag = CMap_LayerPolicy::LAYER_ENV_EFFECT;

	pOutTargets->pStageObjectTag = L"MapStage";
}

CMap_Loader* CMap_Loader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMap_Loader* pInstance = new CMap_Loader(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Cloned : CMap_Loader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMap_Loader::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pProxy);

	__super::Free();
}

NS_END