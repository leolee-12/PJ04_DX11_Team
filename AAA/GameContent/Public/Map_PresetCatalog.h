#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

class CLIENT_DLL CMap_PresetCatalog final
{
public:
	static _uint Get_Count();
	static const _char* Get_Label(_uint iPresetIndex);
	static HRESULT Get_ManifestPath(_uint iPresetIndex, _wstring* pOutManifestPath);
};

NS_END