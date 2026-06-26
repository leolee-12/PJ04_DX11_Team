#pragma once
#include "Monster_State_Move.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Chase : public CMonster_State_Move
{
protected:
	CMonster_State_Chase() = default;
	virtual ~CMonster_State_Chase() = default;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;

protected:
	virtual void					Apply_Movement(_float fTimeDelta) override;

public:
	static CMonster_State_Chase*	Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
	virtual void					Free() override;
};

NS_END