#pragma once
#include "Monster_State_Detect.h"

NS_BEGIN(Client)

class CCappy_State_Detect final : public CMonster_State_Detect
{
protected:
	CCappy_State_Detect() = default;
	virtual ~CCappy_State_Detect() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

public:
	static CCappy_State_Detect*		Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
	virtual void					Free() override;
};

NS_END