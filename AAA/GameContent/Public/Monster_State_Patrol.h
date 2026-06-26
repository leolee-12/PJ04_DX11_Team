#pragma once
#include "Monster_State_Move.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Patrol final : public CMonster_State_Move
{
protected:
	CMonster_State_Patrol() = default;
	virtual ~CMonster_State_Patrol() = default;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(MONSTER_STATE_TYPE  ePrevState) override;
	virtual void					Update(_float fTimeDelta) override;

protected:
	virtual void					Apply_Movement(_float fTimeDelta) override;

public:
	static CMonster_State_Patrol*	Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

private:
	void							Pick_WanderTarget();
	_vector							Offset_To_XZ(const _float3& vTargetF3) const;

private:
	_float3							m_vWanderTarget = {};

	// BasePos 기준 배회 반경 
	static constexpr _float			s_fPatrolRange = 10.f;
	
	// 도착 판정 거리
	static constexpr _float			s_fArriveDist = 0.5f;

	// 한 배회 최대 시간
	static constexpr _float			s_fMaxWanderTime = 4.f;

protected:
	virtual void					Free() override;
};

NS_END