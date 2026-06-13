#pragma once
#include "GameContent_enum.h"
#include "GameContrnt_Events.h"

#ifdef GAMECONTENT_EXPORTS
#define CLIENT_DLL __declspec(dllexport)
#else
#define CLIENT_DLL __declspec(dllimport)
#endif

namespace Client
{
	enum class LEVEL { STATIC, LOADING, LOGO, LOBBY, GAMEPLAY, END };

	enum class SOUND_CHANNEL : _uint
	{
		BGM = 0, 
		END = 32
	};

	enum class VTXTEX_SHADER { DEFAULT, ALPHABLEND };


	enum class KIRBY_ABILITY_TYPE { NORMAL, SWORD };

	constexpr _uint CL_PLAYER_SENSOR = 0;
	constexpr _uint CL_ENV_TRIGGER = 1;

}

using namespace Client;