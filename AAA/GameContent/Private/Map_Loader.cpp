#include "Map_Loader.h"
#include "Map_PresetCatalog.h"
#include "Map_EditFile.h"
#include "Map_Builder.h"
#include "Map_ModelResolver.h"
#include "Map_ProtoRegister.h"
#include "Map_Spawner.h"
#include "MapGimmickSection.h"
#include "GameContent_Log.h"
#include "LevelDesign_Loader.h"

#include "GameInstance.h"
#include "DataLoader.h"
#include "Texture_Hub.h"

#include <mutex>
#include <filesystem>
#include <memory>

namespace
{
	using namespace std::filesystem;

	static constexpr const _tchar* kLayerMapStage = L"Layer_MapStage";
	static constexpr const _tchar* kLayerEnvStatic = L"Layer_EnvStatic";
	static constexpr const _tchar* kLayerEnvInteract = L"Layer_EnvInteract";
	static constexpr const _tchar* kLayerEnvEffect = L"Layer_EnvEffect";

	constexpr _tchar kMapTexPoolRoot[] = L"../../Resources/Map/TexPool";
	_bool g_bMapTexHubReady = false;
	mutex g_MapTexHubMutex;

	using MAP_PACKAGE_HANDLE = shared_ptr<const MAP_PACKAGE>;

	struct MAP_PACKAGE_CACHE_ENTRY
	{
		MAP_PACKAGE_HANDLE pPackage;
		MAP_PACKAGE_BUILD_OPTIONS BuildOptions;
	};

	mutex g_MapPackageCacheMutex;
	unordered_map<_wstring, MAP_PACKAGE_CACHE_ENTRY> g_MapPackageCache;

	_bool Is_EnvPickingModelProtoTag(const _wstring& strModelProtoTag)
	{
		static const _wstring kSuffix = L"__pick";
		if (strModelProtoTag.length() < kSuffix.length())
			return false;

		return 0 == strModelProtoTag.compare(
			strModelProtoTag.length() - kSuffix.length(),
			kSuffix.length(),
			kSuffix);
	}

	_wstring Make_EnvPickingModelProtoTag(const _wstring& strModelProtoTag)
	{
		if (strModelProtoTag.empty())
			return strModelProtoTag;

		if (Is_EnvPickingModelProtoTag(strModelProtoTag))
			return strModelProtoTag;

		return strModelProtoTag + L"__pick";
	}

	void Apply_EnvPickingModelTags(MAP_PACKAGE* pPackage)
	{
		if (nullptr == pPackage)
			return;

		for (ENV_OBJECT_DESC& Desc : pPackage->EnvObjectDescs)
		{
			if (Desc.wstrModelProtoTag.empty())
				continue;

			Desc.wstrModelProtoTag = Make_EnvPickingModelProtoTag(Desc.wstrModelProtoTag);
		}
	}

	_bool Is_EnvLayerInternal(const _wstring& strLayerTag)
	{
		return strLayerTag == kLayerEnvStatic
			|| strLayerTag == kLayerEnvInteract
			|| strLayerTag == kLayerEnvEffect;
	}

	_bool Is_MapLayerInternal(const _wstring& strLayerTag)
	{
		return strLayerTag == kLayerMapStage
			|| strLayerTag == CMapGimmickSection::LAYER_TAG
			|| Is_EnvLayerInternal(strLayerTag);
	}

	_bool Is_MapStageLayerInternal(const _wstring& strLayerTag)
	{
		return strLayerTag == kLayerMapStage
			|| strLayerTag == CMapGimmickSection::LAYER_TAG;
	}

	string To_LogPath(const path& FilePath)
	{
		return WstrToStr(FilePath.wstring());
	}

	_wstring Make_MapCacheKey(const _wstring& strManifestPath, _uint iRuntimeLevel)
	{
		return to_wstring(iRuntimeLevel) + L"|" + strManifestPath;
	}

	MAP_PACKAGE_BUILD_OPTIONS Make_PackageBuildOptions(const MAP_LOAD_OPTIONS& Options)
	{
		MAP_PACKAGE_BUILD_OPTIONS BuildOptions{};
		BuildOptions.bBuildStage = Options.bLoadStage;
		BuildOptions.bBuildEnv = Options.bLoadEnv;
		BuildOptions.bBuildLevelDesignPaths = Options.bLoadLevelDesign;
		BuildOptions.bApplyDelta = true;
		return BuildOptions;
	}

	MAP_EDIT_APPLY_OPTIONS Make_PackageApplyOptions(const MAP_LOAD_OPTIONS& Options)
	{
		MAP_EDIT_APPLY_OPTIONS ApplyOptions{};
		ApplyOptions.bApplyStage = Options.bLoadStage;
		ApplyOptions.bApplyEnv = Options.bLoadEnv;
		ApplyOptions.bApplyAddedObjects = Options.bLoadEnv || Options.bLoadLevelDesign;
		return ApplyOptions;
	}

	_bool Has_PackageBuildScope(
		const MAP_PACKAGE_BUILD_OPTIONS& Cached,
		const MAP_PACKAGE_BUILD_OPTIONS& Required)
	{
		return (!Required.bBuildStage || Cached.bBuildStage)
			&& (!Required.bBuildEnv || Cached.bBuildEnv)
			&& (!Required.bBuildLevelDesignPaths || Cached.bBuildLevelDesignPaths);
	}

	void Store_MapPackage(
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		const MAP_PACKAGE_BUILD_OPTIONS& BuildOptions,
		MAP_PACKAGE&& Package)
	{
		MAP_PACKAGE_HANDLE pPackage = make_shared<MAP_PACKAGE>(move(Package));

		lock_guard<mutex> Lock(g_MapPackageCacheMutex);
		g_MapPackageCache[Make_MapCacheKey(strManifestPath, iRuntimeLevel)] = {
				pPackage,
				BuildOptions
		};
	}

	_bool Try_GetMapPackage(
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		const MAP_PACKAGE_BUILD_OPTIONS& RequiredBuildOptions,
		MAP_PACKAGE_HANDLE* pOutPackage)
	{
		if (nullptr == pOutPackage)
			return false;

		lock_guard<mutex> Lock(g_MapPackageCacheMutex);

		const auto Iter = g_MapPackageCache.find(Make_MapCacheKey(strManifestPath, iRuntimeLevel));
		if (Iter == g_MapPackageCache.end())
			return false;

		if (!Has_PackageBuildScope(Iter->second.BuildOptions, RequiredBuildOptions))
			return false;

		*pOutPackage = Iter->second.pPackage;
		return true;
	}

	bool Try_LoadLevelMapContent(const _wstring& strLevelObjectsPath, MAP_EDIT_DATA* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return false;

		*pOutDesc = {};

		if (strLevelObjectsPath.empty())
			return false;

		_string strContent{};
		if (FAILED(CDataLoader::Read_Json(strLevelObjectsPath.c_str(), &strContent)))
			return false;

		try
		{
			json jLevel = json::parse(strContent);
			return SUCCEEDED(CMap_EditFile::Load_Data(jLevel, pOutDesc));
		}
		catch (const json::exception&)
		{
			return false;
		}
	}

	bool Is_RuntimeLoadContextValid(const MAP_RUNTIME_LOAD_CONTEXT& Context)
	{
		return nullptr != Context.pDevice
			&& nullptr != Context.pContext;
	}

	void Collect_DeletedEnvDescs(
		const vector<ENV_OBJECT_DESC>& SourceDescs,
		const MAP_EDIT_CHANGE* pOverrideDesc,
		vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs)
	{
		if (nullptr == pOutDeletedEnvDescs)
			return;

		pOutDeletedEnvDescs->clear();

		if (nullptr == pOverrideDesc)
			return;

		for (const auto& Desc : SourceDescs)
		{
			const _wstring strKey = CMap_EditFile::Make_EnvKey(Desc);
			if (pOverrideDesc->DeletedEnvObjectKeys.find(strKey) != pOverrideDesc->DeletedEnvObjectKeys.end())
				pOutDeletedEnvDescs->push_back(Desc);
		}
	}

	void Build_RuntimeStageLevels(const MAP_RUNTIME_LOAD_CONTEXT& Context, MAP_RUNTIME_LEVELS* pOutLevels)
	{
		if (nullptr == pOutLevels)
			return;

		*pOutLevels = {};
		pOutLevels->iObjectLevel = ETOUI(LEVEL::STATIC);
		pOutLevels->iStageModelLevel = Context.iModelLevel;
		pOutLevels->iEnvModelLevel = ETOUI(LEVEL::STATIC);
	}

	void Build_RuntimeEnvLevels(const MAP_RUNTIME_LOAD_CONTEXT& Context, _bool bEnableEnvObjectPicking, MAP_RUNTIME_LEVELS* pOutLevels)
	{
		if (nullptr == pOutLevels)
			return;

		*pOutLevels = {};
		pOutLevels->iObjectLevel = ETOUI(LEVEL::STATIC);
		pOutLevels->iStageModelLevel = ETOUI(LEVEL::STATIC);
		pOutLevels->iEnvModelLevel = ETOUI(LEVEL::STATIC);
		pOutLevels->bEnableEnvObjectPicking = bEnableEnvObjectPicking;
	}

	HRESULT Resolve_LevelMapRequest(
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_wstring* pOutManifestPath,
		MAP_EDIT_DATA* pOutMapContentDesc,
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

		const HRESULT hrAsset = CMap_EditFile::Load_EditFile(
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

HRESULT CMap_Loader::Load_FromManifest(
	const _wstring& strManifestPath,
	const MAP_RUNTIME_LEVELS& Levels,
	const MAP_SPAWN_TARGETS& Targets,
	const MAP_LOAD_OPTIONS& Options,
	MAP_LOAD_RESULT* pOutReport,
	CMapStage** ppOutStage)
{
	if (!Options.bLoadStage
		&& !Options.bLoadEnv
		&& !Options.bLoadLevelDesign)
	{
		return E_FAIL;
	}

	const MAP_PACKAGE_BUILD_OPTIONS BuildOptions = Make_PackageBuildOptions(Options);

	MAP_PACKAGE Package{};

	HRESULT hr = Build_Package(strManifestPath, BuildOptions, &Package);
	if (FAILED(hr))
		return hr;

	hr = Ready_Prototypes(Levels, Package);
	if (FAILED(hr))
		return hr;

	MAP_SPAWN_REQUEST Request{};
	Request.Levels = Levels;
	Request.Targets = Targets;
	Request.Options.bSpawnStage = Options.bLoadStage;
	Request.Options.bSpawnEnv = Options.bLoadEnv;
	Request.Options.bSpawnAddedEnv = Options.bLoadEnv;
	Request.Options.bSpawnAddedLevelDesign = Options.bLoadLevelDesign;
	Request.ppOutStage = ppOutStage;

	if (Options.bLoadStage || Options.bLoadEnv || Options.bLoadLevelDesign)
	{
		hr = Spawn(Package, Request, pOutReport);
		if (FAILED(hr))
			return hr;
	}

	if (Options.bLoadLevelDesign)
		hr = Load_LevelDesignEntries(Package, Request, pOutReport);

	return hr;
}

HRESULT CMap_Loader::Preload_LevelDesignEntries(const MAP_PACKAGE& Package, const MAP_RUNTIME_LEVELS& Levels)
{
	if (Package.LevelDesignJsonPaths.empty())
		return S_OK;

	if (nullptr == m_pDevice || nullptr == m_pContext || nullptr == m_pProxy)
		return E_FAIL;

	if (FAILED(Ready_TexHub(m_pProxy)))
		return E_FAIL;

	for (const _wstring& strJsonPath : Package.LevelDesignJsonPaths)
	{
		if (strJsonPath.empty())
			continue;

		LD_RUNTIME_LEVELS LDLevels{};
		LDLevels.iObjectLevel = Levels.iLevelDesignObjectLevel;
		LDLevels.iPrototypeLevel = Levels.iLevelDesignPrototypeLevel;
		LDLevels.iModelPrototypeLevel = Levels.iLevelDesignModelPrototypeLevel;

		if (FAILED(CLevelDesign_Loader::Preload_LevelDesign(m_pDevice, m_pContext, strJsonPath, LDLevels)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMap_Loader::Load_LevelDesignEntries(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_RESULT* pOutReport, const MAP_EDIT_CHANGE* pOverrideDesc)
{
	if (Package.LevelDesignJsonPaths.empty())
		return S_OK;

	if (nullptr == m_pDevice || nullptr == m_pContext || nullptr == m_pProxy)
		return E_FAIL;

	if (FAILED(Ready_TexHub(m_pProxy)))
		return E_FAIL;

	HRESULT hrFinal = S_OK;

	for (const _wstring& strJsonPath : Package.LevelDesignJsonPaths)
	{
		if (strJsonPath.empty())
			continue;

		LD_RUNTIME_LOAD_CONTEXT Context{};
		Context.pDevice = m_pDevice;
		Context.pContext = m_pContext;
		Context.iPlaceLevel = Request.Levels.iLevelDesignObjectLevel;
		Context.iPrototypeLevel = Request.Levels.iLevelDesignPrototypeLevel;
		Context.iModelPrototypeLevel = Request.Levels.iLevelDesignModelPrototypeLevel;
		Context.pCreatedCallback = Request.pCreatedCallback;
		Context.pCallbackContext = Request.pCallbackContext;

		LD_LOAD_RESULT LDReport{};
		const HRESULT hrLoad = CLevelDesign_Loader::Load_LevelDesign_Runtime(Context, strJsonPath, &LDReport, pOverrideDesc);
		if (FAILED(hrLoad))
			return hrLoad;
		if (S_FALSE == hrLoad)
			hrFinal = S_FALSE;

		if (nullptr != pOutReport)
		{
			++pOutReport->iLevelDesignJsonLoadedCount;
			pOutReport->iLevelDesignParsedObjectCount += LDReport.iParsedObjectCount;
			pOutReport->iLevelDesignCreatedCount += LDReport.iCreatedCount;
			pOutReport->iLevelDesignFallbackSpecCount += LDReport.iFallbackSpecCount;
			pOutReport->iLevelDesignSkippedCreateFailedCount += LDReport.iSkippedCreateFailedCount;
		}
	}

	return hrFinal;
}

HRESULT CMap_Loader::Build_Package(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage)
{
	return Build_Package(strManifestPath, MAP_PACKAGE_BUILD_OPTIONS{}, pOutPackage);
}

HRESULT CMap_Loader::Build_Package(const _wstring& strManifestPath, const MAP_PACKAGE_BUILD_OPTIONS& BuildOptions, MAP_PACKAGE* pOutPackage)
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

	const HRESULT hr = pBuilder->Build_FromManifest(strManifestPath, BuildOptions, pOutPackage);

	Safe_Release(pBuilder);
	Safe_Release(pResolver);

	return hr;
}

HRESULT CMap_Loader::Ready_Prototypes(const MAP_RUNTIME_LEVELS& Levels, const MAP_PACKAGE& Package)
{
	if (nullptr == m_pDevice || nullptr == m_pContext || nullptr == m_pProxy)
		return E_FAIL;

	if (FAILED(Ready_TexHub(m_pProxy)))
		return E_FAIL;

	CMap_ProtoRegister* pRegister = CMap_ProtoRegister::Create(m_pDevice, m_pContext);
	if (nullptr == pRegister)
		return E_FAIL;

	const HRESULT hr = pRegister->Ready_Prototypes(Levels, Package);

	Safe_Release(pRegister);

	return hr;
}

HRESULT CMap_Loader::Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_RESULT* pOutReport)
{
	CMap_Spawner* pSpawner = CMap_Spawner::Create();
	if (nullptr == pSpawner)
		return E_FAIL;

	const HRESULT hr = pSpawner->Spawn(Package, Request, pOutReport);

	Safe_Release(pSpawner);

	return hr;
}

HRESULT CMap_Loader::Preload_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	const _wstring& strManifestPath, _uint iRuntimeLevel, const MAP_LOAD_OPTIONS& Options)
{
	if (nullptr == pDevice || nullptr == pContext || strManifestPath.empty())
		return E_FAIL;

	if (!Options.bLoadStage && !Options.bLoadEnv && !Options.bLoadLevelDesign)
	{
		return E_FAIL;
	}

	CMap_Loader* pMapLoader = Create(pDevice, pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	const MAP_PACKAGE_BUILD_OPTIONS BuildOptions = Make_PackageBuildOptions(Options);

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strManifestPath, BuildOptions, &Package);

	if (SUCCEEDED(hr))
	{
		MAP_RUNTIME_LEVELS Levels{};
		Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);

		Levels.bEnableEnvObjectPicking =
			Options.bEnableEnvObjectPicking;

		if (Options.bLoadStage || Options.bLoadEnv || !Package.AddedObjectDescs.empty())
			hr = pMapLoader->Ready_Prototypes(Levels, Package);

		if (SUCCEEDED(hr) && Options.bLoadLevelDesign)
		{
			hr = pMapLoader->Preload_LevelDesignEntries(Package, Levels);
		}
	}

	if (SUCCEEDED(hr))
		Store_MapPackage(strManifestPath, iRuntimeLevel, BuildOptions, move(Package));

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Preload_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	const _wstring& strFallbackManifestPath, const _wstring& strLevelObjectsPath, _uint iRuntimeLevel, const MAP_LOAD_OPTIONS& Options)
{
	if (nullptr == pDevice || nullptr == pContext)
		return E_FAIL;

	if (!Options.bLoadStage && !Options.bLoadEnv && !Options.bLoadLevelDesign)
	{
		return E_FAIL;
	}

	_wstring strResolvedManifestPath;
	MAP_EDIT_DATA MapContentDesc{};
	if (FAILED(Resolve_LevelMapRequest(strFallbackManifestPath, strLevelObjectsPath, &strResolvedManifestPath, &MapContentDesc)))
	{
		return E_FAIL;
	}

	CMap_Loader* pMapLoader = Create(pDevice, pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	const MAP_PACKAGE_BUILD_OPTIONS BuildOptions = Make_PackageBuildOptions(Options);

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strResolvedManifestPath, BuildOptions, &Package);

	MAP_PACKAGE PreloadPackage{};
	if (SUCCEEDED(hr))
	{
		PreloadPackage = Package;

		if (MapContentDesc.bHasMapContent)
		{
			hr = CMap_EditFile::Apply_Change(&PreloadPackage, MapContentDesc.OverrideDesc, Make_PackageApplyOptions(Options));
		}
	}

	if (SUCCEEDED(hr))
	{
		MAP_RUNTIME_LEVELS Levels{};
		Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);

		Levels.bEnableEnvObjectPicking = Options.bEnableEnvObjectPicking;

		if (Options.bLoadStage || Options.bLoadEnv || !PreloadPackage.AddedObjectDescs.empty())
			hr = pMapLoader->Ready_Prototypes(Levels, PreloadPackage);

		if (SUCCEEDED(hr) && Options.bLoadLevelDesign)
		{
			hr = pMapLoader->Preload_LevelDesignEntries(PreloadPackage, Levels);
		}
	}

	if (SUCCEEDED(hr))
		Store_MapPackage(strResolvedManifestPath, iRuntimeLevel, BuildOptions, move(Package));

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Spawn_Map(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const _wstring& strManifestPath,
	_uint iRuntimeLevel,
	MAP_LOAD_RESULT* pOutReport,
	CMapStage** ppOutStage,
	const MAP_LOAD_OPTIONS& Options,
	const MAP_EDIT_CHANGE* pLevelDesignOverrideDesc,
	const MAP_EDIT_CHANGE* pMapOverrideDesc)
{
	if (nullptr == pDevice || nullptr == pContext || strManifestPath.empty())
		return E_FAIL;

	if (!Options.bLoadStage
		&& !Options.bLoadEnv
		&& !Options.bLoadLevelDesign)
	{
		return E_FAIL;
	}

	const MAP_PACKAGE_BUILD_OPTIONS RequiredBuildOptions = Make_PackageBuildOptions(Options);

	MAP_PACKAGE_HANDLE pCachedPackage;
	if (!Try_GetMapPackage(
		strManifestPath,
		iRuntimeLevel,
		RequiredBuildOptions,
		&pCachedPackage))
	{
		return E_FAIL;
	}

	MAP_PACKAGE OverridePackage{};
	const MAP_PACKAGE* pSpawnPackage = pCachedPackage.get();

	if (nullptr != pMapOverrideDesc)
	{
		OverridePackage = *pCachedPackage;

		if (FAILED(CMap_EditFile::Apply_Change(
			&OverridePackage,
			*pMapOverrideDesc,
			Make_PackageApplyOptions(Options))))
		{
			return E_FAIL;
		}

		pSpawnPackage = &OverridePackage;
	}

	CMap_Loader* pMapLoader = Create(pDevice, pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_RUNTIME_LEVELS Levels{};
	MAP_SPAWN_TARGETS Targets{};
	Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);
	Build_DefaultRuntimeTargets(iRuntimeLevel, &Targets);

	Levels.bEnableEnvObjectPicking = Options.bEnableEnvObjectPicking;

	MAP_SPAWN_REQUEST Request{};
	Request.Levels = Levels;
	Request.Targets = Targets;
	Request.Options.bSpawnStage = Options.bLoadStage;
	Request.Options.bSpawnEnv = Options.bLoadEnv;
	Request.Options.bSpawnAddedEnv = Options.bLoadEnv;
	Request.Options.bSpawnAddedLevelDesign = Options.bLoadLevelDesign;
	Request.ppOutStage = ppOutStage;

	HRESULT hr = S_OK;

	if (nullptr != pMapOverrideDesc
		&& (Options.bLoadStage || Options.bLoadEnv || !pSpawnPackage->AddedObjectDescs.empty()))
	{
		hr = pMapLoader->Ready_Prototypes(Levels, *pSpawnPackage);
		if (FAILED(hr))
		{
			Safe_Release(pMapLoader);
			return hr;
		}
	}

	if (Options.bLoadStage || Options.bLoadEnv || Options.bLoadLevelDesign)
		hr = pMapLoader->Spawn(*pSpawnPackage, Request, pOutReport);

	if (SUCCEEDED(hr) && Options.bLoadLevelDesign)
	{
		hr = pMapLoader->Load_LevelDesignEntries(
			*pSpawnPackage,
			Request,
			pOutReport,
			pLevelDesignOverrideDesc);
	}

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Spawn_Map(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const _wstring& strFallbackManifestPath,
	const _wstring& strLevelObjectsPath,
	_uint iRuntimeLevel,
	MAP_LOAD_RESULT* pOutReport,
	CMapStage** ppOutStage,
	const MAP_LOAD_OPTIONS& Options)
{
	if (nullptr == pDevice || nullptr == pContext)
		return E_FAIL;

	_wstring strResolvedManifestPath;
	MAP_EDIT_DATA MapContentDesc{};
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

	const MAP_EDIT_CHANGE* pLevelDesignOverrideDesc = MapContentDesc.bHasMapContent ? &MapContentDesc.OverrideDesc : nullptr;

	CMapStage* pLocalStage = nullptr;
	CMapStage** ppStageForSpawn = ppOutStage;

	if (Options.bLoadStage
		&& nullptr == ppStageForSpawn
		&& bHasMapStageOverride)
	{
		ppStageForSpawn = &pLocalStage;
	}

	const HRESULT hrSpawn = Spawn_Map(
		pDevice,
		pContext,
		strResolvedManifestPath,
		iRuntimeLevel,
		pOutReport,
		ppStageForSpawn,
		Options,
		pLevelDesignOverrideDesc,
		pLevelDesignOverrideDesc);

	if (FAILED(hrSpawn))
		return hrSpawn;

	if (Options.bLoadStage && bHasMapStageOverride)
	{
		CMapStage* pStageToApply =
			nullptr != ppOutStage ? *ppOutStage : pLocalStage;

		if (FAILED(CMap_EditFile::Apply_Stage(
			pStageToApply,
			jMapStageOverride)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
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

	MAP_EDIT_DATA MapContentDesc{};
	json jLocalMapStageOverride = json::object();
	_bool bLocalHasMapStageOverride = false;

	json* pStageOverride = nullptr != pOutMapStageOverride ? pOutMapStageOverride : &jLocalMapStageOverride;
	_bool* pHasStageOverride = nullptr != pOutHasMapStageOverride ? pOutHasMapStageOverride : &bLocalHasMapStageOverride;

	const HRESULT hrAsset = CMap_EditFile::Load_EditFile(
		strMapManifestPath,
		&MapContentDesc,
		pStageOverride,
		pHasStageOverride);

	if (FAILED(hrAsset) && S_FALSE != hrAsset)
		return hrAsset;

	CMap_Loader* pMapLoader = Create(Context.pDevice, Context.pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_PACKAGE_BUILD_OPTIONS BuildOptions{};
	BuildOptions.bBuildStage = true;
	BuildOptions.bBuildEnv = false;
	BuildOptions.bBuildLevelDesignPaths = false;
	BuildOptions.bApplyDelta = false;

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strMapManifestPath, BuildOptions, &Package);

	if (SUCCEEDED(hr) && MapContentDesc.bHasMapContent)
	{
		MAP_EDIT_APPLY_OPTIONS ApplyOptions{};
		ApplyOptions.bApplyStage = true;
		ApplyOptions.bApplyEnv = false;
		ApplyOptions.bApplyAddedObjects = false;
		hr = CMap_EditFile::Apply_Change(&Package, MapContentDesc.OverrideDesc, ApplyOptions);
	}

	if (SUCCEEDED(hr))
	{
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
			Request.Options.bSpawnAddedEnv = false;
			Request.Options.bSpawnAddedLevelDesign = false;
			Request.pCreatedCallback = Context.pCreatedCallback;
			Request.pCallbackContext = Context.pCallbackContext;
			Request.ppOutStage = ppStageForSpawn;

			hr = pMapLoader->Spawn(Package, Request, nullptr);

			if (SUCCEEDED(hr) && *pHasStageOverride)
			{
				CMapStage* pStageToApply = nullptr != ppOutStage ? *ppOutStage : pLocalStage;
				hr = CMap_EditFile::Apply_Stage(pStageToApply, *pStageOverride);
			}
		}
	}

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Load_Env_Runtime(
	const MAP_RUNTIME_LOAD_CONTEXT& Context,
	const _wstring& strMapManifestPath,
	const MAP_EDIT_CHANGE* pOverrideDesc,
	MAP_LOAD_RESULT* pOutReport,
	vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs,
	_bool bEnableEnvObjectPicking)
{
	if (nullptr != pOutReport)
		*pOutReport = {};

	if (nullptr != pOutDeletedEnvDescs)
		pOutDeletedEnvDescs->clear();

	if (!Is_RuntimeLoadContextValid(Context) || strMapManifestPath.empty())
		return E_FAIL;

	MAP_EDIT_DATA LoadedMapContentDesc{};
	const MAP_EDIT_CHANGE* pResolvedOverrideDesc = pOverrideDesc;

	if (nullptr == pResolvedOverrideDesc)
	{
		const HRESULT hrAsset = CMap_EditFile::Load_EditFile(
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

	MAP_PACKAGE_BUILD_OPTIONS BuildOptions{};
	BuildOptions.bBuildStage = false;
	BuildOptions.bBuildEnv = true;
	BuildOptions.bBuildLevelDesignPaths = false;
	BuildOptions.bApplyDelta = false;

	MAP_PACKAGE SourcePackage{};
	HRESULT hr = pMapLoader->Build_Package(strMapManifestPath, BuildOptions, &SourcePackage);

	if (SUCCEEDED(hr))
	{
		Collect_DeletedEnvDescs(SourcePackage.EnvObjectDescs, pResolvedOverrideDesc, pOutDeletedEnvDescs);

		MAP_PACKAGE SpawnPackage = SourcePackage;

		if (nullptr != pResolvedOverrideDesc)
		{
			MAP_EDIT_APPLY_OPTIONS ApplyOptions{};
			ApplyOptions.bApplyStage = false;
			ApplyOptions.bApplyEnv = true;
			ApplyOptions.bApplyAddedObjects = true;
			hr = CMap_EditFile::Apply_Change(&SpawnPackage, *pResolvedOverrideDesc, ApplyOptions);
		}

		if (SUCCEEDED(hr) && bEnableEnvObjectPicking)
		{
			Apply_EnvPickingModelTags(&SpawnPackage);
		}

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
				Request.Options.bSpawnAddedEnv = true;
				Request.Options.bSpawnAddedLevelDesign = false;
				Request.pCreatedCallback = Context.pCreatedCallback;
				Request.pCallbackContext = Context.pCallbackContext;
				Request.ppOutEnvInstanceController = Context.ppOutEnvInstanceController;

				hr = pMapLoader->Spawn(SpawnPackage, Request, pOutReport);
			}
		}
	}

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Load_LevelDesign_Runtime(const MAP_RUNTIME_LOAD_CONTEXT& Context, const _wstring& strMapManifestPath, MAP_LOAD_RESULT*
	pOutReport, const MAP_EDIT_CHANGE* pOverrideDesc)
{
	if (nullptr != pOutReport)
		*pOutReport = {};

	if (!Is_RuntimeLoadContextValid(Context) || strMapManifestPath.empty())
		return E_FAIL;

	MAP_EDIT_DATA LoadedMapContentDesc{};
	const MAP_EDIT_CHANGE* pResolvedOverrideDesc = pOverrideDesc;

	if (nullptr == pResolvedOverrideDesc)
	{
		const HRESULT hrAsset = CMap_EditFile::Load_EditFile(strMapManifestPath, &LoadedMapContentDesc);
		if (FAILED(hrAsset) && S_FALSE != hrAsset)
			return hrAsset;

		if (LoadedMapContentDesc.bHasMapContent)
			pResolvedOverrideDesc = &LoadedMapContentDesc.OverrideDesc;
	}

	CMap_Loader* pMapLoader = Create(Context.pDevice, Context.pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_PACKAGE_BUILD_OPTIONS BuildOptions{};
	BuildOptions.bBuildStage = false;
	BuildOptions.bBuildEnv = false;
	BuildOptions.bBuildLevelDesignPaths = true;
	BuildOptions.bApplyDelta = false;

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strMapManifestPath, BuildOptions, &Package);

	if (SUCCEEDED(hr) && nullptr != pResolvedOverrideDesc)
	{
		MAP_EDIT_APPLY_OPTIONS ApplyOptions{};
		ApplyOptions.bApplyStage = false;
		ApplyOptions.bApplyEnv = false;
		ApplyOptions.bApplyAddedObjects = true;
		hr = CMap_EditFile::Apply_Change(&Package, *pResolvedOverrideDesc, ApplyOptions);
	}

	if (SUCCEEDED(hr))
	{
		MAP_RUNTIME_LEVELS Levels{};
		Levels.iObjectLevel = ETOUI(LEVEL::STATIC);
		Levels.iStageModelLevel = ETOUI(LEVEL::STATIC);
		Levels.iEnvModelLevel = ETOUI(LEVEL::STATIC);
		Levels.iLevelDesignObjectLevel = Context.iPlaceLevel;
		Levels.iLevelDesignPrototypeLevel = Context.iPlaceLevel;
		Levels.iLevelDesignModelPrototypeLevel = Context.iModelLevel;

		MAP_SPAWN_REQUEST Request{};
		Request.Levels = Levels;
		Request.Options.bSpawnStage = false;
		Request.Options.bSpawnEnv = false;
		Request.Options.bSpawnAddedEnv = false;
		Request.Options.bSpawnAddedLevelDesign = true;
		Request.pCreatedCallback = Context.pCreatedCallback;
		Request.pCallbackContext = Context.pCallbackContext;

		hr = pMapLoader->Ready_Prototypes(Levels, Package);

		if (SUCCEEDED(hr))
			hr = pMapLoader->Spawn(Package, Request, pOutReport);

		if (SUCCEEDED(hr))
			hr = pMapLoader->Load_LevelDesignEntries(Package, Request, pOutReport, pResolvedOverrideDesc);
	}

	Safe_Release(pMapLoader);
	return hr;
}

HRESULT CMap_Loader::Collect_PreloadJobs(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strFallbackManifestPath,
	const _wstring& strLevelObjectsPath, _uint iRuntimeLevel, vector<function<HRESULT()>>* pOutJobs)
{
	if (nullptr == pDevice || nullptr == pContext || nullptr == pOutJobs)
		return E_FAIL;

	_wstring strResolvedManifestPath;
	MAP_EDIT_DATA MapContentDesc{};
	if (FAILED(Resolve_LevelMapRequest(strFallbackManifestPath, strLevelObjectsPath,
		&strResolvedManifestPath, &MapContentDesc)))
		return E_FAIL;

	CMap_Loader* pMapLoader = Create(pDevice, pContext);
	if (nullptr == pMapLoader)
		return E_FAIL;

	MAP_PACKAGE Package{};
	HRESULT hr = pMapLoader->Build_Package(strResolvedManifestPath, &Package);
	Safe_Release(pMapLoader);

	MAP_PACKAGE PreloadPackage{};
	if (SUCCEEDED(hr))
	{
		PreloadPackage = Package;
		if (MapContentDesc.bHasMapContent)
			hr = CMap_EditFile::Apply_Change(&PreloadPackage, MapContentDesc.OverrideDesc);
	}

	if (FAILED(hr))
		return hr;

	MAP_RUNTIME_LEVELS Levels{};
	Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);

	{
		vector<MAP_ADD_OBJECT> Added = PreloadPackage.AddedObjectDescs;
		pOutJobs->push_back([pDevice, pContext, Added, Levels]() -> HRESULT
			{ return Preload_SharedObjects(pDevice, pContext, Added, Levels); });
	}

	vector<MAP_SECTION_DESC> SectionDescs = PreloadPackage.StageDesc.SectionDescs;
	Append_MapGimmickSectionDescs(PreloadPackage.StageDesc.strStageName, Levels.iStageModelLevel, &SectionDescs);

	for (const MAP_SECTION_DESC& Desc : SectionDescs)
		pOutJobs->push_back([pDevice, pContext, Desc, Levels]() -> HRESULT
			{ return Preload_One_SectionModel(pDevice, pContext, Desc, Levels); });

	unordered_map<_wstring, pair<ENV_OBJECT_DESC, _bool>> EnvModelJobs;
	for (const ENV_OBJECT_DESC& Desc : PreloadPackage.EnvObjectDescs)
	{
		if (Desc.wstrModelProtoTag.empty())
			continue;

		const _bool bCook = Desc.tCollision.eColliderKind == ENV_COLLIDER_KIND::MODEL_MESH
			&& Desc.tCollision.bCookCollMesh;

		auto iter = EnvModelJobs.find(Desc.wstrModelProtoTag);
		if (iter == EnvModelJobs.end())
			EnvModelJobs.emplace(Desc.wstrModelProtoTag, make_pair(Desc, bCook));
		else
			iter->second.second = iter->second.second || bCook;
	}
	for (const auto& Pair : EnvModelJobs)
	{
		const ENV_OBJECT_DESC Desc = Pair.second.first;
		const _bool bCook = Pair.second.second;
		pOutJobs->push_back([pDevice, pContext, Desc, bCook, Levels]() -> HRESULT
			{ return Preload_One_EnvModel(pDevice, pContext, Desc, bCook, Levels); });
	}

	for (const _wstring& strJsonPath : PreloadPackage.LevelDesignJsonPaths)
	{
		if (strJsonPath.empty())
			continue;

		pOutJobs->push_back([pDevice, pContext, strJsonPath, Levels]() -> HRESULT
			{
				CGameInstance_Proxy* pProxy = CGameInstance::GetProxy();
				const HRESULT hrHub = Ready_TexHub(pProxy);
				Safe_Release(pProxy);
				if (FAILED(hrHub))
					return E_FAIL;

				LD_RUNTIME_LEVELS LDLevels{};
				LDLevels.iObjectLevel = Levels.iLevelDesignObjectLevel;
				LDLevels.iPrototypeLevel = Levels.iLevelDesignPrototypeLevel;
				LDLevels.iModelPrototypeLevel = Levels.iLevelDesignModelPrototypeLevel;
				return CLevelDesign_Loader::Preload_LevelDesign(pDevice, pContext, strJsonPath, LDLevels);
			});
	}

	Store_MapPackage(strResolvedManifestPath, iRuntimeLevel, MAP_PACKAGE_BUILD_OPTIONS{}, move(Package));

	return S_OK;
}

_uint CMap_Loader::Get_MapCount()
{
	return CMap_PresetCatalog::Get_Count();
}

const _char* CMap_Loader::Get_MapName(_uint iMapIndex)
{
	return CMap_PresetCatalog::Get_Label(iMapIndex);
}

HRESULT CMap_Loader::Get_MapManifestPath(_uint iMapIndex, _wstring* pOutManifestPath)
{
	return CMap_PresetCatalog::Get_ManifestPath(iMapIndex, pOutManifestPath);
}

_bool CMap_Loader::Is_MapLayer(const _wstring& strLayerTag)
{
	return Is_MapLayerInternal(strLayerTag);
}

_bool CMap_Loader::Is_MapStageLayer(const _wstring& strLayerTag)
{
	return Is_MapStageLayerInternal(strLayerTag);
}

_bool CMap_Loader::Is_MapEnvLayer(const _wstring& strLayerTag)
{
	return Is_EnvLayerInternal(strLayerTag);
}

HRESULT CMap_Loader::Get_EditFilePath(const _wstring& strManifestPath, _wstring* pOutEditFilePath)
{
	return CMap_EditFile::Get_EditFilePath(strManifestPath, pOutEditFilePath);
}

HRESULT CMap_Loader::Load_EditFile(const _wstring& strManifestPath, MAP_EDIT_DATA* pInOutData, json* pOutStageEdit, _bool* pOutHasStageEdit)
{
	return CMap_EditFile::Load_EditFile(strManifestPath, pInOutData, pOutStageEdit, pOutHasStageEdit);
}

HRESULT CMap_Loader::Save_EditFile(const MAP_EDIT_DATA& Data, const CMapStage* pStage)
{
	return CMap_EditFile::Save_EditFile(Data, pStage);
}

HRESULT CMap_Loader::Get_PresetEditFilePath(_uint iMapIndex, const _wstring& strManifestPath, _wstring* pOutEditFilePath)
{
	return CMap_EditFile::Get_PresetEditFilePath(iMapIndex, strManifestPath, pOutEditFilePath);
}

HRESULT CMap_Loader::Load_PresetEditFile(_uint iMapIndex, const _wstring& strManifestPath,
	MAP_EDIT_DATA* pInOutData, json* pOutStageEdit, _bool* pOutHasStageEdit)
{
	return CMap_EditFile::Load_PresetEditFile(iMapIndex, strManifestPath, pInOutData, pOutStageEdit, pOutHasStageEdit);
}

HRESULT CMap_Loader::Save_PresetEditFile(_uint iMapIndex, const MAP_EDIT_DATA& Data, const CMapStage* pStage)
{
	return CMap_EditFile::Save_PresetEditFile(iMapIndex, Data, pStage);
}

HRESULT CMap_Loader::Ready_TexHub(CGameInstance_Proxy* pProxy)
{
	if (nullptr == pProxy || !pProxy->IsConnected())
	{
		Log_GameContentError("Ready_TexHub failed: invalid proxy.");
		return E_FAIL;
	}

	lock_guard<mutex> Lock(g_MapTexHubMutex);

	if (g_bMapTexHubReady)
		return S_OK;

	unordered_map<wstring, path> TextureID;

	error_code ErrorCode;
	const path Root(kMapTexPoolRoot);
	if (!exists(Root, ErrorCode) || ErrorCode)
	{
		Log_GameContentError(
			"Ready_TexHub failed: TexPool root missing or inaccessible. root="
			+ To_LogPath(Root));
		return E_FAIL;
	}

	for (recursive_directory_iterator Iter(
		Root,
		directory_options::skip_permission_denied,
		ErrorCode), End;
		Iter != End;
		Iter.increment(ErrorCode))
	{
		if (ErrorCode)
		{
			Log_GameContentError(
				"Ready_TexHub failed: directory iteration error. root="
				+ To_LogPath(Root));
			break;
		}

		if (!Iter->is_regular_file())
			continue;

		const path FilePath = Iter->path();
		wstring strExt = FilePath.extension().wstring();
		transform(strExt.begin(), strExt.end(), strExt.begin(),
			[](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });

		if (L".dds" != strExt && L".png" != strExt)
			continue;

		const wstring strTextureId =
			Engine::CTexture_Hub::Normalize_TextureName(FilePath.filename().wstring());

		const auto iter = TextureID.find(strTextureId);
		if (iter == TextureID.end())
		{
			TextureID.emplace(strTextureId, FilePath);
			continue;
		}

		const bool bExistingDDS = 0 == _wcsicmp(iter->second.extension().c_str(), L".dds");
		const bool bCurrentDDS = 0 == _wcsicmp(FilePath.extension().c_str(), L".dds");

		if (bExistingDDS == bCurrentDDS)
		{
			Log_GameContentError(
				"Ready_TexHub failed: duplicate texture id. id="
				+ WstrToStr(strTextureId)
				+ " existing="
				+ To_LogPath(iter->second)
				+ " incoming="
				+ To_LogPath(FilePath));
			return E_FAIL;
		}

		if (bCurrentDDS)
			iter->second = FilePath;
	}

	if (ErrorCode)
		return E_FAIL;

	for (const auto& Pair : TextureID)
	{
		TEXTURE_HANDLE Handle = INVALID_TEXTURE_HANDLE;
		const HRESULT hrLoad = pProxy->LoadOrGet_TextureFromHub(Pair.second.c_str(), &Handle);
		if (FAILED(hrLoad))
		{
			Log_GameContentError(
				"Ready_TexHub failed: texture load failed. id="
				+ WstrToStr(Pair.first)
				+ " path="
				+ To_LogPath(Pair.second));
			return hrLoad;
		}

		const HRESULT hrRegister = pProxy->Register_TextureNameInHub(Pair.first.c_str(), Handle);
		if (FAILED(hrRegister))
		{
			Log_GameContentError(
				"Ready_TexHub failed: texture name registration failed. id="
				+ WstrToStr(Pair.first)
				+ " path="
				+ To_LogPath(Pair.second));
			return hrRegister;
		}
	}

	g_bMapTexHubReady = true;
	return S_OK;
}

void CMap_Loader::Build_DefaultRuntimeLevels(_uint iRuntimeLevel, MAP_RUNTIME_LEVELS* pOutLevels)
{
	if (nullptr == pOutLevels)
		return;

	*pOutLevels = {};
	pOutLevels->iObjectLevel = ETOUI(LEVEL::STATIC);
	pOutLevels->iStageModelLevel = ETOUI(LEVEL::STATIC);
	pOutLevels->iEnvModelLevel = ETOUI(LEVEL::STATIC);
	pOutLevels->iLevelDesignObjectLevel = iRuntimeLevel;
	pOutLevels->iLevelDesignPrototypeLevel = iRuntimeLevel;
	pOutLevels->iLevelDesignModelPrototypeLevel = iRuntimeLevel;
}

void CMap_Loader::Build_DefaultRuntimeTargets(_uint iRuntimeLevel, MAP_SPAWN_TARGETS* pOutTargets)
{
	if (nullptr == pOutTargets)
		return;

	*pOutTargets = {};

	pOutTargets->Stage.iPlaceLevel = iRuntimeLevel;
	pOutTargets->Stage.pLayerTag = kLayerMapStage;

	pOutTargets->EnvStatic.iPlaceLevel = iRuntimeLevel;
	pOutTargets->EnvStatic.pLayerTag = kLayerEnvStatic;

	pOutTargets->EnvInteract.iPlaceLevel = iRuntimeLevel;
	pOutTargets->EnvInteract.pLayerTag = kLayerEnvInteract;

	pOutTargets->EnvEffect.iPlaceLevel = iRuntimeLevel;
	pOutTargets->EnvEffect.pLayerTag = kLayerEnvEffect;

	pOutTargets->pStageObjectTag = L"MapStage";
}

HRESULT CMap_Loader::Preload_SharedObjects(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	const vector<MAP_ADD_OBJECT>& AddedDescs, const MAP_RUNTIME_LEVELS& Levels)
{
	CGameInstance_Proxy* pProxy = CGameInstance::GetProxy();
	const HRESULT hrHub = Ready_TexHub(pProxy);
	Safe_Release(pProxy);

	if (FAILED(hrHub))
		return E_FAIL;

	CMap_ProtoRegister* pRegister = CMap_ProtoRegister::Create(pDevice, pContext);
	if (nullptr == pRegister)
		return E_FAIL;

	HRESULT hr = pRegister->Ready_ObjectPrototypes(Levels.iObjectLevel);

	if (SUCCEEDED(hr))
		hr = pRegister->Ready_AddedObjectPrototypes(Levels, AddedDescs);

	Safe_Release(pRegister);
	return hr;
}

HRESULT CMap_Loader::Preload_One_SectionModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const MAP_SECTION_DESC& Desc, const MAP_RUNTIME_LEVELS& Levels)
{
	CGameInstance_Proxy* pProxy = CGameInstance::GetProxy();
	const HRESULT hrHub = Ready_TexHub(pProxy);
	Safe_Release(pProxy);
	if (FAILED(hrHub))
		return E_FAIL;

	CMap_ProtoRegister* pRegister = CMap_ProtoRegister::Create(pDevice, pContext);
	if (nullptr == pRegister)
		return E_FAIL;

	const HRESULT hr = pRegister->Ready_MapSectionModel(Levels.iStageModelLevel, Desc);
	Safe_Release(pRegister);
	return hr;
}

HRESULT CMap_Loader::Preload_One_EnvModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const ENV_OBJECT_DESC& Desc, _bool bCookCollisionMesh, const MAP_RUNTIME_LEVELS& Levels)
{
	CGameInstance_Proxy* pProxy = CGameInstance::GetProxy();
	const HRESULT hrHub = Ready_TexHub(pProxy);
	Safe_Release(pProxy);
	if (FAILED(hrHub))
		return E_FAIL;

	CMap_ProtoRegister* pRegister = CMap_ProtoRegister::Create(pDevice, pContext);
	if (nullptr == pRegister)
		return E_FAIL;

	const HRESULT hr = pRegister->Ready_EnvModel(
		Levels.iEnvModelLevel, Desc, bCookCollisionMesh, Levels.bEnableEnvObjectPicking);
	Safe_Release(pRegister);

	if (FAILED(hr))
	{
		Log_GameContentWarning("EnvObject model skipped: object="
			+ WstrToStr(Desc.wstrObjectName) + " path=" + WstrToStr(Desc.wstrModelPath));
	}
	return S_OK;
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
