#pragma once
#include "Monster_State_Attack.h"

NS_BEGIN(Client)
class CBladeKnight;

class CBladeKnight_State_DoubleAttack final : public CMonster_State_Attack
{
private:
	CBladeKnight_State_DoubleAttack() = default;
	virtual ~CBladeKnight_State_DoubleAttack() = default;

public:
	virtual MONSTER_STATE_TYPE			Get_StateType() override;
	virtual MONSTER_STATE_TYPE			Get_NextState() override;

protected:
	virtual void						Exit(MONSTER_STATE_TYPE eNextState) override;

protected:
	virtual void						Play_AttackAnimation() override;

public:
	static CBladeKnight_State_DoubleAttack* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);


protected:
	virtual void						Free() override;
};

NS_END