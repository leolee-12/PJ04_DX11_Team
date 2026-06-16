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
	RETREAT,

	// 중간 인터럽트 상태
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

	_bool					bActionFinished = { false }; // 현재 상태 액션의 행동 완료 신호
};

NS_END