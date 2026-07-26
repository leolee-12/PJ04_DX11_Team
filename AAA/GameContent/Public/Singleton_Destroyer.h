#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

class CLIENT_DLL CSingleton_Destroyer
{
public:
	CSingleton_Destroyer() = default;
	~CSingleton_Destroyer() = default;

public:
	static void Destroy_GameContent_Singletons();
	static void Clear_LevelRegisteries(_uint iLevel);
};

NS_END

