#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Client)

struct MAP_OVERRIDE_DESC
{
	_uint Version = 2;
	unordered_set<_wstring> DeletedEnvObjectKeys;
	vector<MAP_ADDED_OBJECT_DESC> AddedMapObjects;
};

class CLIENT_DLL CMap_Override final
{
public:
	static _wstring Build_EnvObjectStableKey(const ENV_OBJECT_DESC& Desc);

	static json Serialize(const MAP_OVERRIDE_DESC& Desc);
	static HRESULT Deserialize(const json& jOverride, MAP_OVERRIDE_DESC* pOutDesc);

	static HRESULT Apply(MAP_PACKAGE* pInOutPackage, const MAP_OVERRIDE_DESC& OverrideDesc);
};

NS_END