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
	enum class LEVEL { STATIC, FIRST_LOADING, LOADING, STAGE0_STEP1, STAGE0_STEP2, TOWN_STEP1, BOSS_STAGE1, 
		STAGE1_STEP1, STAGE1_STEP2, STAGE1_STEP3, TEST, TOWN_STEP2, END };

	enum class VTXTEX_SHADER { DEFAULT, ALPHABLEND };

	enum class KIRBY_SHADER_PASS { KIRBY, ANIM_TEXTURED_PBR, ANIM_CONSTANT_PBR, ANIM_SHADOW, BOMBHAT, };
	enum class COPY_ABILITY_TYPE { NONE, NORMAL, SWORD, BOMB, ICE };
	enum class DEFORM_TYPE { NONE, CAR, CYLINDER, COASTER };
	
	static constexpr _float s_fQTE_GorillaLimit = { 5.f };

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
		CAR_BOOST,
		ENV_HURT,
		ENV_FOLIAGE,
		ENV_LADDER,
		ESSENCE_BUBBLE,
		DROPPED_BUBBLE,
		DEFORM_OBJECT,
		DROP_STAR,
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