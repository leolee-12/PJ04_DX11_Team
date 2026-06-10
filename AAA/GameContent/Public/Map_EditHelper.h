#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"
#include "Map_Override.h"
#include "Map_LevelContent.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)
class CMapStage;

class CLIENT_DLL CMap_EditHelper final
{
public:
    struct MAP_PRESET_LOAD_REPORT
    {
        _bool           bStageLoaded = { false };
        _wstring        strStageName = {};
        _uint           iEnvJsonLoadedCount = {};
        _uint           iEnvJsonFailedCount = {};
        _uint           iEnvDescriptorCount = {};
        _uint           iEnvCreatedCount = {};
        _uint           iEnvSkippedMissingModel = {};
        _uint           iEnvSkippedCreateFailed = {};
    };

    using MAP_PRESET_OBJECT_CREATED_CALLBACK = MAP_OBJECT_CREATED_CALLBACK;

public:
    static _bool Is_MapLayer(const wstring& strLayerTag);
    static _uint Get_MapPresetCount();
    static const _char* Get_MapPresetLabel(_uint iPresetIndex);
    static HRESULT Get_MapPresetManifestPath(_uint iPresetIndex, _wstring* pOutManifestPath);

    static HRESULT Get_MapOverrideAssetPath(
        const _wstring& strManifestPath,
        _wstring* pOutOverridePath);

    static HRESULT Load_MapOverrideAsset(
        const _wstring& strManifestPath,
        MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc,
        json* pOutMapStageOverride = nullptr,
        _bool* pOutHasMapStageOverride = nullptr);

    static HRESULT Save_MapOverrideAsset(
        const MAP_LEVEL_CONTENT_DESC& MapContentDesc,
        const CMapStage* pStage);

    static HRESULT Get_MapPresetOverrideAssetPath(
        _uint iPresetIndex,
        const _wstring& strManifestPath,
        _wstring* pOutOverridePath);

    static HRESULT Load_MapPresetOverrideAsset(
        _uint iPresetIndex,
        const _wstring& strManifestPath,
        MAP_LEVEL_CONTENT_DESC* pInOutMapContentDesc,
        json* pOutMapStageOverride = nullptr,
        _bool* pOutHasMapStageOverride = nullptr);

    static HRESULT Save_MapPresetOverrideAsset(
        _uint iPresetIndex,
        const MAP_LEVEL_CONTENT_DESC& MapContentDesc,
        const CMapStage* pStage);

    static HRESULT Load_MapStage(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iPresetIndex,
        _uint iPlaceLevel,
        _uint iModelLevel,
        MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
        void* pCallbackContext = nullptr,
        CMapStage** ppOutStage = nullptr,
        _wstring* pOutStageName = nullptr);

    static HRESULT Load_EnvObject_FromJson(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iPresetIndex,
        _uint iPlaceLevel,
        MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
        void* pCallbackContext = nullptr,
        MAP_PRESET_LOAD_REPORT* pOutReport = nullptr);

    static HRESULT Load_MapPreset(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iPresetIndex,
        _uint iPlaceLevel,
        _uint iModelLevel,
        MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
        void* pCallbackContext = nullptr,
        MAP_PRESET_LOAD_REPORT* pOutReport = nullptr,
        CMapStage** ppOutStage = nullptr);

    static HRESULT Import_EnvJson_Immediate(
        CGameInstance_Proxy* pProxy,
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iObjectLevel,
        const _wstring& strJsonPath,
        const _wstring& strStageFolderName,
        MAP_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
        void* pCallbackContext = nullptr,
        MAP_LOAD_REPORT* pOutReport = nullptr);

    static HRESULT Load_MapPreset_WithOverride(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iPresetIndex,
        const _wstring& strManifestPath,
        _uint iPlaceLevel,
        _uint iModelLevel,
        const MAP_OVERRIDE_DESC* pOverrideDesc = nullptr,
        MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
        void* pCallbackContext = nullptr,
        MAP_PRESET_LOAD_REPORT* pOutReport = nullptr,
        CMapStage** ppOutStage = nullptr,
        vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs = nullptr,
        _wstring* pOutResolvedManifestPath = nullptr);

    static HRESULT Load_PresetEnv_WithOverride(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iPresetIndex,
        const _wstring& strManifestPath,
        _uint iPlaceLevel,
        const MAP_OVERRIDE_DESC* pOverrideDesc = nullptr,
        MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
        void* pCallbackContext = nullptr,
        MAP_PRESET_LOAD_REPORT* pOutReport = nullptr,
        vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs = nullptr,
        _wstring* pOutResolvedManifestPath = nullptr);

    static json Serialize_MapStageOverride(const CMapStage* pStage);
    static HRESULT Apply_MapStageOverride(CMapStage* pStage, const json& jOverride);
};

NS_END