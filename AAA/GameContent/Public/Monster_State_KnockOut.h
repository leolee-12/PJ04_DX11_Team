#pragma once
#include "Monster_State_Death.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_KnockOut : public CMonster_State_Death
{
protected:
	CMonster_State_KnockOut() = default;
	virtual ~CMonster_State_KnockOut() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;
	virtual void					Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;
public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;

protected:
	virtual void					Apply_DeathLaunch(_fvector vAttackerPos, _float fStrength) override;

public:
	static CMonster_State_KnockOut* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
	virtual void					Free() override;
};

NS_END