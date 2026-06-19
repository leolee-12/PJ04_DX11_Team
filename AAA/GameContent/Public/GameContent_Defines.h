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
	enum class LEVEL { STATIC, FIRST_LOADING, LOADING, GAMEPLAY, TEST, END };

	enum class SOUND_CHANNEL : _uint
	{
		BGM = 0, 
		END = 32
	};

	enum class VTXTEX_SHADER { DEFAULT, ALPHABLEND };


	enum class KIRBY_ABILITY_TYPE { NORMAL, SWORD };

	enum class COLLISION_LAYER : _uint 
	{ 
		PLAYER_HURT, 
		PLAYER_HIT,
		PLAYER_INHALE,
		PLAYER_PROJECTILE,
		MONSTER_HURT,
		MONSTER_HIT,
		MONSTER_PROJECTILE,
		MONSTER_D_RANGE,
		ENV_TRIGGER = 100
	};

	constexpr _uint KIRBY_SILHOUETTE_ID = 200;
}

using namespace Client;