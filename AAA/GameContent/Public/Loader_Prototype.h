#pragma once
#include "Engine_Defines.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

HRESULT CLIENT_DLL Ready_Prototype_SharedResources(CGameInstance_Proxy* pProxy,
	ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

HRESULT CLIENT_DLL Ready_Prototype_Shaders(CGameInstance_Proxy* pProxy,
	ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

HRESULT CLIENT_DLL Load_Level(
    CGameInstance_Proxy* pProxy,
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    const _tchar* strFilePath,
    _uint iLevelIndex);

HRESULT CLIENT_DLL Load_Fonts(CGameInstance_Proxy* pProxy);

NS_END