#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CCappy_State_Idle : public CMonster_State
{
protected:
	CCappy_State_Idle() = default;
	virtual ~CCappy_State_Idle() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

public:
	static CCappy_State_Idle*		Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

private:
	void							Play_IdleAnimation();


protected:
	virtual void					Free() override;
};

NS_END