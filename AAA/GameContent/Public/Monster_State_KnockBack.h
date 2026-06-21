#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_KnockBack  : public CMonster_State
{
protected:
	CMonster_State_KnockBack() = default;
	virtual ~CMonster_State_KnockBack() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter() override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

public:
	static CMonster_State_KnockBack* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);


protected:
	virtual void					Free() override;
};

NS_END