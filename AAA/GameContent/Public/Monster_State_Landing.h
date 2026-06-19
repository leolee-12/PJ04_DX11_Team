#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CMonster_State_Landing final : public CMonster_State
{
private:
	CMonster_State_Landing();
	virtual ~CMonster_State_Landing() = default;

private:
	HRESULT							Initialize();

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(CMonster* pMonster) override;
	virtual void					Update(CMonster* pMonster, _float fTimeDelta) override;
	virtual void					Exit(CMonster* pMonster) override;

public:
	static CMonster_State_Landing* Create();

protected:
	virtual void					Free() override;
};

NS_END