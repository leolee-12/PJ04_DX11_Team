#pragma once
#include "Monster_Brain.h"

NS_BEGIN(Client)

class CMonster;

class CMonster_Brain_FSM final : public CMonsterBrain
{
private:
	CMonster_Brain_FSM();
	virtual ~CMonster_Brain_FSM() = default;

private:
	HRESULT						Initialize();

public: 
	virtual void				Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
	static CMonster_Brain_FSM*	Create();

protected:
	virtual void				Free() override;
};

NS_END
