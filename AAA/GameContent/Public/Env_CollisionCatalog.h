#pragma once

#include "GameContent_Defines.h"

NS_BEGIN(Client)

struct ENV_COLLISION_CATALOG_RECORD
{
	wstring wstrObjectName;
	wstring strApxbinName;
	wstring strBfresPath;
};

class CLIENT_DLL CEnv_CollisionCatalog final
{
public:
	static HRESULT Load(const _wstring& strCatalogPath);
	static void Clear();

	static _bool Is_Loaded();
	static _bool Try_Find(const _wstring& wstrObjectName, ENV_COLLISION_CATALOG_RECORD* pOutRecord = nullptr);
};

NS_END