#pragma once
#include "Base.h"
#include "Map_Override.h"

NS_BEGIN(Client)

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

class CLIENT_DLL CMap_LevelContent final
{
public:
    static json Serialize(const MAP_EDIT_DATA& Desc);
    static HRESULT Deserialize(const json& jLevelRoot, MAP_EDIT_DATA* pOutDesc);
};

NS_END