#pragma once

#include "EnvObject_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CEnvObjectLoader final
{
public:
	static HRESULT Ready_ObjectPrototypes(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iObjectLevel);
	static HRESULT Build_Descriptors_FromJsonFile(const wstring& strJsonPath, vector<ENV_OBJECT_DESC>* pOutDescs);
	static HRESULT Load_FromJsonFile(
		CGameInstance_Proxy* pProxy,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		_uint iObjectLevel,
		const wstring& strJsonPath);
};

NS_END
