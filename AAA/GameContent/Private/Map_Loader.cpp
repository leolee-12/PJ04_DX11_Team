#include "Map_Loader.h"
#include "Map_LevelContent.h"
#include "Map_EditHelper.h"
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
    std::unordered_map<std::wstring, Client::MAP_PACKAGE> g_MapPackageCache;

    std::wstring Make_MapCacheKey(const std::wstring& strManifestPath, _uint iRuntimeLevel)
    {
        return std::to_wstring(iRuntimeLevel) + L"|" + strManifestPath;
    }

    void Store_MapPackage(
        const std::wstring& strManifestPath,
        _uint iRuntimeLevel,
        const Client::MAP_PACKAGE& Package)
    {
        std::lock_guard<std::mutex> lock(g_MapPackageCacheMutex);
        g_MapPackageCache[Make_MapCacheKey(strManifestPath, iRuntimeLevel)] = Package;
    }

    bool Try_GetMapPackage(
        const std::wstring& strManifestPath,
        _uint iRuntimeLevel,
        Client::MAP_PACKAGE* pOutPackage)
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

        const HRESULT hrAsset = Client::CMap_EditHelper::Load_MapOverrideAsset(
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

HRESULT CMap_Loader::Preload_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strManifestPath, _uint
    iRuntimeLevel)
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

HRESULT CMap_Loader::Preload_MapForLevel(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    const _wstring& strFallbackManifestPath,
    const _wstring& strLevelObjectsPath,
    _uint iRuntimeLevel)
{
    _wstring strResolvedManifestPath;
    MAP_LEVEL_CONTENT_DESC MapContentDesc{};
    if (FAILED(Resolve_LevelMapRequest(
        strFallbackManifestPath,
        strLevelObjectsPath,
        &strResolvedManifestPath,
        &MapContentDesc)))
    {
        return E_FAIL;
    }

    return Preload_Map(
        pDevice,
        pContext,
        strResolvedManifestPath,
        iRuntimeLevel);
}

HRESULT CMap_Loader::Spawn_Map(const _wstring& strManifestPath, _uint iRuntimeLevel,
    MAP_LOAD_REPORT* pOutReport, CMapStage** ppOutStage)
{
    return Spawn_Map_WithOverride(
        strManifestPath,
        iRuntimeLevel,
        nullptr,
        pOutReport,
        ppOutStage);
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

HRESULT CMap_Loader::Spawn_MapForLevel(
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

    const MAP_OVERRIDE_DESC* pOverrideDesc = nullptr;
    if (MapContentDesc.bHasMapContent)
        pOverrideDesc = &MapContentDesc.OverrideDesc;

    CMapStage* pLocalStage = nullptr;
    CMapStage** ppStageForSpawn = ppOutStage;

    if (nullptr == ppStageForSpawn && bHasMapStageOverride)
        ppStageForSpawn = &pLocalStage;

    const HRESULT hrSpawn = Spawn_Map_WithOverride(
        strResolvedManifestPath,
        iRuntimeLevel,
        pOverrideDesc,
        pOutReport,
        ppStageForSpawn);

    if (FAILED(hrSpawn))
        return hrSpawn;

    CMapStage* pStageToApply = nullptr;
    if (nullptr != ppOutStage)
        pStageToApply = *ppOutStage;
    else
        pStageToApply = pLocalStage;

    if (bHasMapStageOverride)
    {
        if (FAILED(CMap_EditHelper::Apply_MapStageOverride(
            pStageToApply,
            jMapStageOverride)))
        {
            return E_FAIL;
        }
    }

    return S_OK;
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
    pOutTargets->Stage.pLayerTag = L"Layer_MapStage";

    pOutTargets->EnvStatic.iPlaceLevel = iRuntimeLevel;
    pOutTargets->EnvStatic.pLayerTag = L"Layer_EnvStatic";

    pOutTargets->EnvInteract.iPlaceLevel = iRuntimeLevel;
    pOutTargets->EnvInteract.pLayerTag = L"Layer_EnvInteract";

    pOutTargets->EnvEffect.iPlaceLevel = iRuntimeLevel;
    pOutTargets->EnvEffect.pLayerTag = L"Layer_EnvEffect";

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