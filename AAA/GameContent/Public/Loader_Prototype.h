#pragma once
#include "Engine_Defines.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

struct ENV_ENTRY 
{ 
    const _tchar* tag; 
    const _tchar* diff; 
    const _tchar* spec; 
    float intensity; 
};

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

HRESULT CLIENT_DLL Ready_Prototype_UIPartObjects(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

// (LEVEL_NAME)_ui.json : manifest 파일을 읽고 해당하는 레벨 Prototype Table에 등록한다. -> Loader에서 호출해야함
HRESULT CLIENT_DLL Ready_Level_UIResources(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex);

// (LEVEL_NAME)_ui.json : manifest 파일을 읽고 해당하는 레벨의 Object Table에 배치한다(ContainerObject만 - PartObject들은 Prototype 찾아서 Container가 갖고 있음)
HRESULT CLIENT_DLL Load_Level_UI(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex);

NS_END