#pragma once
#include "Monster_State_Attack.h"

NS_BEGIN(Client)
class CBladeKnight;

class CBladeKnight_State_Attack final : public CMonster_State_Attack
{
private:
	CBladeKnight_State_Attack() = default;
	virtual ~CBladeKnight_State_Attack() = default;

protected:
	virtual HRESULT						Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE			Get_StateType() override;

public:
	virtual void						Exit(MONSTER_STATE_TYPE eNextState) override;

protected:
	virtual void						Play_AttackAnimation() override;

public:
	static CBladeKnight_State_Attack*	Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
	virtual void						Free() override;
};

NS_END