#pragma once
#include "Monster_State_Idle.h"

NS_BEGIN(Client)

class CPoppyBrosJr_State_Idle final : public CMonster_State_Idle
{
protected:
	CPoppyBrosJr_State_Idle() = default;
	virtual ~CPoppyBrosJr_State_Idle() = default;

public:
	virtual void					Update(_float fTimeDelta) override;

public:
	static CPoppyBrosJr_State_Idle* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
	virtual void					Free() override;
};

NS_END