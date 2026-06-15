#pragma once

#include "GameContent_Defines.h"

#include <cfloat>

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

enum class MONSTER_STATE_TYPE
{
	IDLE,
	CHASE,
	ATTACK,
	HIT,
	CAPTURED,
	DEAD,
};

struct MONSTER_BLACKBOARD
{
	_bool					bCanSeeTarget = { false };		

	_float					fDistToTarget = { FLT_MAX };
	
	Engine::CGameObject*	pTarget = { nullptr };

	_float3					vTargetPos = {};
	_float3					vLastKnownPos = {};
};

NS_END