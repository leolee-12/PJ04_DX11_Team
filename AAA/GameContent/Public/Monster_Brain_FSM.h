#pragma once
#include "Monster_Brain.h"

NS_BEGIN(Client)

class CMonster;

class CMonster_Brain_FSM : public CMonsterBrain
{
protected:
	CMonster_Brain_FSM();
	virtual ~CMonster_Brain_FSM() = default;

public: 
	virtual void				Decide(CMonster*, const MONSTER_BLACKBOARD&, _float) override;

protected:
	HRESULT						Initialize();
	_bool						Can_Decide(CMonster*, const MONSTER_BLACKBOARD&) const;
	virtual void				Decide_Combat(CMonster*, const MONSTER_BLACKBOARD&, _float) {}

public:
	static CMonster_Brain_FSM*	Create();

private:
	_bool						m_bSpotted = { false };		// IDLE -> FIND 전이를 위한 변수

protected:
	virtual void				Free() override;
};

NS_END
