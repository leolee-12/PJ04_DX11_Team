#pragma once

#include "EnvObject_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

struct ENV_OBJECT_LOAD_REPORT
{
	_uint iDescriptorCount = {};
	_uint iCreatedCount = {};
	_uint iSkippedMissingModel = {};
	_uint iSkippedCreateFailed = {};
};

using ENV_OBJECT_CREATED_CALLBACK = void(*)(void* pContext, CGameObject* pObject, const wstring& strPrototypeTag, const wstring& strLayerTag, const wstring& strObjectTag);

class CLIENT_DLL CEnvObjectLoader final
{
public:
	static HRESULT Ready_ObjectPrototypes(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iObjectLevel);
    static HRESULT Build_Descriptors_FromJsonFile(
        const wstring& strJsonPath,
        const wstring& strStageFolderName,
        vector<ENV_OBJECT_DESC>* pOutDescs);

    static HRESULT Load_FromJsonFile(
        CGameInstance_Proxy* pProxy,
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iObjectLevel,
        const wstring& strJsonPath,
        const wstring& strStageFolderName);

    static HRESULT Load_FromJsonFile(
        CGameInstance_Proxy* pProxy,
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iObjectLevel,
        const wstring& strJsonPath,
        const wstring& strStageFolderName,
        ENV_OBJECT_CREATED_CALLBACK pCreatedCallback,
        void* pCallbackContext,
        ENV_OBJECT_LOAD_REPORT* pOutReport = nullptr);
};

NS_END
