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
	vector<_wstring> LevelDesignJsonPaths;
	_wstring strDeltaPath;
	_wstring strDecorCollisionCatalogPath;
};

struct MAP_ADD_OBJECT
{
	_wstring strPrototypeTag;
	_wstring strLayerTag;
	_wstring strObjectTag;
	json jObject;
};

struct MAP_ENV_EDITED_DESC
{
	_wstring strStableKey;

	_bool bHasRenderable = { false };
	_bool bRenderable = { true };

	_bool bHasEnableCulling = { false };
	_bool bEnableCulling = { true };

	_bool bHasCastShadow = { false };
	_bool bCastShadow = { false };

	_bool bHasWorldMatrix = { false };
	_float4x4 matWorld = {};

	_bool bDisableCollMesh = { false };
	// Legacy read compatibility only.

	_bool bHasCollMeshEdited = { false };
	_bool bCreateCollMesh = { true };

	_bool bHasNearDistAlpha = { false };
	_bool bUseNearDistAlpha = { false };
};

struct MAP_EDIT_CHANGE
{
	_uint Version = 3;
	unordered_set<_wstring> DeletedEnvObjectKeys;
	unordered_map<_wstring, MAP_ENV_EDITED_DESC> EditedEnvObjects;
	unordered_map<_wstring, MAP_ENV_EDITED_DESC> EditedMapSections;
	vector<MAP_ADD_OBJECT> AddedMapObjects;
};

struct MAP_EDIT_DATA
{
	_bool bHasMapContent = false;
	_uint Version = 1;
	_int iPresetIndex = -1;
	_wstring strManifestPath;
	_bool bLoadStage = true;
	_bool bLoadEnv = true;
	MAP_EDIT_CHANGE OverrideDesc;
};

struct MAP_PACKAGE
{
	MAP_STAGE_DESC StageDesc;
	vector<ENV_OBJECT_DESC> EnvObjectDescs;
	vector<_wstring> EnvJsonPaths;
	vector<_wstring> LevelDesignJsonPaths;
	vector<MAP_ADD_OBJECT> AddedObjectDescs;
};

struct MAP_RUNTIME_LEVELS
{
	_uint iObjectLevel = {};
	_uint iStageModelLevel = {};
	_uint iEnvModelLevel = {};

	_uint iLevelDesignObjectLevel = {};
	_uint iLevelDesignPrototypeLevel = {};

	_bool bEnableEnvObjectPicking = { false };
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

struct MAP_LOAD_RESULT
{
	_bool bStageLoaded = false;
	_wstring strStageName;

	_uint iSectionCount = {};
	_uint iEnvJsonLoadedCount = {};
	_uint iEnvDescriptorCount = {};
	_uint iEnvCreatedCount = {};
	_uint iEnvSkippedMissingModel = {};
	_uint iEnvSkippedCreateFailed = {};

	_uint iLevelDesignJsonLoadedCount = {};
	_uint iLevelDesignParsedObjectCount = {};
	_uint iLevelDesignCreatedCount = {};
	_uint iLevelDesignFallbackSpecCount = {};
	_uint iLevelDesignSkippedCreateFailedCount = {};
};

inline _bool Has_AnyMapEnvEdit(const MAP_ENV_EDITED_DESC& Edit)
{
	return Edit.bHasRenderable
		|| Edit.bHasEnableCulling
		|| Edit.bHasCastShadow
		|| Edit.bHasWorldMatrix
		|| Edit.bHasNearDistAlpha
		|| Edit.bHasCollMeshEdited
		|| Edit.bDisableCollMesh;
}

NS_END