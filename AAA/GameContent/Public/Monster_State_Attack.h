#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Attack abstract : public CMonster_State
{
protected:
	CMonster_State_Attack() = default;
	virtual ~CMonster_State_Attack() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual void					Enter() override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

protected:
	virtual void					Play_AttackAnimation() = 0;

protected:
	virtual void					Free() override;
};

NS_END