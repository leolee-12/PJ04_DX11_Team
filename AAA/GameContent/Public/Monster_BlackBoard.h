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

	JUMP,
	FALL,
	LANDING,

	// 중간 인터럽트 상태
	HIT,
	CAPTURED,
	DEAD
};

struct MONSTER_BLACKBOARD
{
	_bool					bCanSeeTarget = { false };		

	_float					fDistToTarget = { FLT_MAX };		// 3D 거리
	_float					fDistToTargetXZ = { FLT_MAX };		// XZ 평면 거리
	_float					fHeightToTarget = {};				// 높이 차이
	
	CGameObject*			pTarget = { nullptr };

	_float3					vTargetPos = {};
	_float3					vLastKnownPos = {};
	_float3					vDirToTargetXZ = {};

	_bool					bMoveLocked = { false };		// AnimEvent에서 LockMove로 움직임 멈춰야되는 시점 지시
	_bool					bActionFinished = { false }; // Non-Loop 액션이 끝났는지?
	_bool					bCanTransition = { true };	// Brain이 일반 상태 전이를 해도 되는지?
};

NS_END