#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CMonster;

class CLIENT_DLL CMonster_State_Attack final : public CMonster_State
{
private:
	CMonster_State_Attack();
	virtual ~CMonster_State_Attack() = default;

public:
	HRESULT							Initialize();

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(CMonster* pMonster) override;
	virtual void					Update(CMonster* pMonster, _float fTimeDelta) override;
	virtual void					Exit(CMonster* pMonster) override;

public:
	static CMonster_State_Attack*	Create();

protected:
	virtual void					Free() override;
};

NS_END