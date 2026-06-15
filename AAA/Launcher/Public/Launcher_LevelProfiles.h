#pragma once
#include "Launcher_Defines.h"

NS_BEGIN(Client)

struct LAUNCHER_LEVEL_PROFILES final
{
	static constexpr const _tchar* LEVEL_FIRSTLOADING =
		L"../../Resources/Level_Manifest/Level_FirstLoading.json";
	static constexpr const _tchar* LEVEL_LOADING =
		L"../../Resources/Level_Manifest/Level_Loading.json";
	static constexpr const _tchar* LEVEL_TEST =
		L"../../Resources/Level_Manifest/Level_Test.json";
};

NS_END