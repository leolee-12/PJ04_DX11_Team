#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CMonster_State_Death abstract : public CMonster_State
{
protected:
	CMonster_State_Death() = default;
	virtual ~CMonster_State_Death() = default;

protected:
	virtual HRESULT		Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual void		Enter() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual void		Exit(MONSTER_STATE_TYPE eNextState) override;

protected:
	virtual void		Apply_DeathLaunch(_fvector vAttackerPos, _float fStrength) = 0;

protected:
	virtual void		Free() override;
};

NS_END