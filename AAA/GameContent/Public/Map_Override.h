#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Client)

struct MAP_EDIT_CHANGE
{
	_uint Version = 2;
	unordered_set<_wstring> DeletedEnvObjectKeys;
	vector<MAP_ADD_OBJECT> AddedMapObjects;
};

class CLIENT_DLL CMap_Override final
{
public:
	static _wstring Build_EnvObjectStableKey(const ENV_OBJECT_DESC& Desc);

	static json Serialize(const MAP_EDIT_CHANGE& Desc);
	static HRESULT Deserialize(const json& jOverride, MAP_EDIT_CHANGE* pOutDesc);
	static HRESULT Apply(MAP_PACKAGE* pInOutPackage, const MAP_EDIT_CHANGE& OverrideDesc);
};

NS_END