#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

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

    static json Serialize_MapStageOverride(const CMapStage* pStage);
    static HRESULT Apply_MapStageOverride(CMapStage* pStage, const json& jOverride);
};

NS_END