#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Engine) 
class CGameInstance_Proxy; 
NS_END

NS_BEGIN(Client)

struct LEVEL_LIGHT
{
    _float4 vDir;        // g_vLightDir
    _float4 vDiffuse;    // g_vLightDiffuse
    _float4 vAmbient;    // g_vLightAmbient
    _float4 vSpecular;   // g_vLightSpecular
    const _tchar* pEnvTag;
};

class CLIENT_DLL CLevelLight_DB final
{
public:
    static const LEVEL_LIGHT& Get(LEVEL eLevel);
    static void Apply(CGameInstance_Proxy* pProxy, LEVEL eLevel);
};

NS_END