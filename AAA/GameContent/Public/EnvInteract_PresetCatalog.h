#pragma once
#include "EnvObject_Defines.h"

NS_BEGIN(Client)

class CEnvInteract_PresetCatalog final
{
private:
	CEnvInteract_PresetCatalog() = delete;
	~CEnvInteract_PresetCatalog() = delete;

public:
	static _bool Try_Find(const _wstring& wstrObjectName, ENV_INTERACT_PRESET* pOutPreset);
};

NS_END