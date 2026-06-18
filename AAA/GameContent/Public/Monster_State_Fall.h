#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CMonster_State_Fall final : public CMonster_State
{
private:
	CMonster_State_Fall();
	virtual ~CMonster_State_Fall() = default;

private:
	HRESULT							Initialize();

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(CMonster* pMonster) override;
	virtual void					Update(CMonster* pMonster, _float fTimeDelta) override;
	virtual void					Exit(CMonster* pMonster) override;

public:
	static CMonster_State_Fall* Create();

protected:
	virtual void					Free() override;
};

NS_END