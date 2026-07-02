#pragma once
#include "GameContent_enum.h"
#include "GameContrnt_Events.h"
#include "GameContent_AnimEvents.h"

#ifdef GAMECONTENT_EXPORTS
#define CLIENT_DLL __declspec(dllexport)
#else
#define CLIENT_DLL __declspec(dllimport)
#endif

namespace Client
{
	enum class LEVEL { STATIC, FIRST_LOADING, LOADING, STAGE0_STEP1, STAGE0_STEP2, TOWN_STEP1, BOSS_STAGE1, TEST, END };

	enum class VTXTEX_SHADER { DEFAULT, ALPHABLEND };

	enum class COPY_ABILITY_TYPE { NONE, NORMAL, SWORD, BOMB };
	enum class DEFORM_TYPE { NONE, CAR };

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
		ENV_HURT,
		ENV_LADDER,
		ENV_TRIGGER = 100
	};

	struct CAPSULE_DESC
	{
		_float3 vCenter = { 0.f, 0.f, 0.f };
		_float  fRadius = { 0.5f };
		_float  fHeight = { 1.f };
		_float3 vRadians = { 0.f, 0.f, 0.f };
	};

	constexpr _uint WORLD_STATIC_ID = 1;
}

using namespace Client;