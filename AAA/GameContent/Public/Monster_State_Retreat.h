#pragma once
#include "Monster_State_Move.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Retreat : public CMonster_State_Move
{
protected:
	CMonster_State_Retreat() = default;
	virtual ~CMonster_State_Retreat() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter() override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

protected:
	virtual void					Apply_Movement(_float fTimeDelta) override;

public:
	static CMonster_State_Retreat*	Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);


protected:
	virtual void					Free() override;
};

NS_END