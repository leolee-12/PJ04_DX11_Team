#pragma once
#include "Map_Defines.h"
#include "EnvObject_Defines.h"
#include "LevelDesign_LoadTypes.h"
#include "Editable.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)
class CMapStage;
class CEnv_InstanceController;

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

struct MAP_PACKAGE_BUILD_OPTIONS
{
	_bool bBuildStage = true;
	_bool bBuildEnv = true;
	_bool bBuildLevelDesignPaths = true;
	_bool bApplyDelta = true;
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

	_bool bHasUseCullDistance = { false };
	_bool bUseCullDistance = { true };

	_bool bHasUseCullFrustum = { false };
	_bool bUseCullFrustum = { true };

	_bool bHasShadow = { false }; // Edit JSON contains UseShadow.
	_bool bUseShadow = { false };

	_bool bHasWorldMatrix = { false };
	_float4x4 matWorld = {};

	_bool bHasCollMesh = { false }; // Edit JSON contains UseCollMesh.
	_bool bUseCollMesh = { false };

	_bool bHasNearDistAlpha = { false };
	_bool bUseNearDistAlpha = { false };
};

struct MAP_SECTION_EDITED_DESC
{
	_wstring strStableKey;

	_bool bHasRenderable = { false };
	_bool bRenderable = { true };

	_bool bHasEnableCulling = { false };
	_bool bEnableCulling = { true };

	_bool bHasWorldMatrix = { false };
	_float4x4 matWorld = {};

	_bool bHasUseCollMesh = { false };
	_bool bUseCollMesh = { false };

	_bool bHasRenderID = { false };
	RENDERID eRenderID = { RENDERID::NONBLEND };
};

struct MAP_LD_EDITED_DESC
{
	_wstring strStableKey;

	_bool bHasWorldMatrix = { false };
	_float4x4 matWorld = {};
};

struct MAP_EDIT_CHANGE
{
	_uint Version = { 6 };
	unordered_set<_wstring> DeletedEnvObjectKeys;
	unordered_set<_wstring> DeletedLevelDesignObjectKeys;
	unordered_map<_wstring, EDIT_OBJECT_OVERRIDE_DESC> EditedEnvObjects;
	unordered_map<_wstring, EDIT_OBJECT_OVERRIDE_DESC> EditedMapSections;
	unordered_map<_wstring, EDIT_OBJECT_OVERRIDE_DESC> EditedLevelDesignObjects;
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
	_uint iEnvSkippedMissingModel = {};
};

struct MAP_RUNTIME_LEVELS
{
	_uint iObjectLevel = {};
	_uint iStageModelLevel = {};
	_uint iEnvModelLevel = {};

	_uint iLevelDesignObjectLevel = {};
	_uint iLevelDesignPrototypeLevel = {};
	_uint iLevelDesignModelPrototypeLevel = {};

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

struct MAP_LOAD_OPTIONS
{
	_bool bLoadStage = true;
	_bool bLoadEnv = true;
	_bool bLoadLevelDesign = true;
	_bool bEnableEnvObjectPicking = false;
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
	CEnv_InstanceController** ppOutEnvInstanceController = nullptr;
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

inline _bool Has_AnyClassOverride(const EDIT_CLASS_OVERRIDE& ClassOverride)
{
	if (const EDIT_ENVOBJECT_OVERRIDE* pEnvOverride = get_if<EDIT_ENVOBJECT_OVERRIDE>(&ClassOverride))
		return pEnvOverride->bHasNearDistAlpha || pEnvOverride->bHasDecalAlpha;

	if (const EDIT_LD_BUSH_OVERRIDE* pBushOverride = get_if<EDIT_LD_BUSH_OVERRIDE>(&ClassOverride))
		return pBushOverride->bHasGenerateItem;

	return false;
}

inline _bool Has_AnyEdit(const EDIT_OBJECT_OVERRIDE_DESC& Edit)
{
	return 0u != Edit.Common.iPolicyMask
		|| Edit.Common.bHasWorldMatrix
		|| Has_AnyClassOverride(Edit.ClassOverride);
}

inline _bool Has_AnyMapEnvEdit(const MAP_ENV_EDITED_DESC& Edit)
{
	return Edit.bHasRenderable
		|| Edit.bHasEnableCulling
		|| Edit.bHasUseCullDistance
		|| Edit.bHasUseCullFrustum
		|| Edit.bHasShadow
		|| Edit.bHasWorldMatrix
		|| Edit.bHasNearDistAlpha
		|| Edit.bHasCollMesh;
}

inline _bool Has_AnyMapSectionEdit(const MAP_SECTION_EDITED_DESC& Edit)
{
	return Edit.bHasRenderable
		|| Edit.bHasEnableCulling
		|| Edit.bHasWorldMatrix
		|| Edit.bHasUseCollMesh
		|| Edit.bHasRenderID;
}

inline _bool Has_AnyMapLDEdit(const MAP_LD_EDITED_DESC& Edit)
{
	return Edit.bHasWorldMatrix;
}

NS_END
