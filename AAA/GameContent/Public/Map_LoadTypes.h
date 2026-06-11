#pragma once
#include "Map_Defines.h"
#include "EnvObject_Defines.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)
class CMapStage;

struct MAP_MANIFEST_DESC
{
    _wstring strLevelName;
    _wstring strStageName;
    _wstring strStageFolderName;
    vector<_wstring> SectionNames;
    vector<MAP_SECTION_TYPE> SectionTypes;
    vector<RENDERID> SectionRenderIDs;
    vector<_wstring> EnvJsonPaths;
    _wstring strDeltaPath;
    _wstring strDecorCollisionCatalogPath;
};

struct MAP_ADDED_OBJECT_DESC
{
    _wstring strPrototypeTag;
    _wstring strLayerTag;
    _wstring strObjectTag;
    json jObject;
};

struct MAP_PACKAGE
{
    MAP_STAGE_DESC StageDesc;
    vector<ENV_OBJECT_DESC> EnvObjectDescs;
    vector<_wstring> EnvJsonPaths;
    vector<MAP_ADDED_OBJECT_DESC> AddedObjectDescs;
};

struct MAP_RUNTIME_LEVELS
{
    _uint iObjectLevel = {};
    _uint iStageModelLevel = {};
    _uint iEnvModelLevel = {};
};

struct MAP_SPAWN_ROUTE
{
    _uint iPlaceLevel = {};
    const _tchar* pLayerTag = nullptr;
};

struct MAP_SPAWN_TARGETS
{
    MAP_SPAWN_ROUTE Stage;
    MAP_SPAWN_ROUTE EnvStatic;
    MAP_SPAWN_ROUTE EnvInteract;
    MAP_SPAWN_ROUTE EnvEffect;

    const _tchar* pStageObjectTag = L"MapStage";
};

struct MAP_SPAWN_OPTIONS
{
    _bool bSpawnStage = true;
    _bool bSpawnEnv = true;
};

using MAP_OBJECT_CREATED_CALLBACK = void(*)(void* pContext, CGameObject* pObject,
    const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);

struct MAP_SPAWN_REQUEST
{
    MAP_RUNTIME_LEVELS Levels;
    MAP_SPAWN_TARGETS Targets;
    MAP_SPAWN_OPTIONS Options;

    MAP_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr;
    void* pCallbackContext = nullptr;
    CMapStage** ppOutStage = nullptr;
};

struct MAP_LOAD_REPORT
{
    _bool bStageLoaded = false;
    _wstring strStageName;

    _uint iSectionCount = {};
    _uint iEnvJsonLoadedCount = {};
    _uint iEnvDescriptorCount = {};
    _uint iEnvCreatedCount = {};
    _uint iEnvSkippedMissingModel = {};
    _uint iEnvSkippedCreateFailed = {};
};

NS_END