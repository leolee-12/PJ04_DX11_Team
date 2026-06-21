#pragma once
#include "Monster_Brain.h"

NS_BEGIN(Client)

class CMonster;

class CMonster_Brain_FSM abstract : public CMonsterBrain
{
protected:
	CMonster_Brain_FSM();
	virtual ~CMonster_Brain_FSM() = default;

public: 
	virtual void				Decide(CMonster*, const MONSTER_BLACKBOARD&, _float) override;

protected:
	virtual HRESULT				Initialize(CMonster* pOwner);
	_bool						Can_Decide(const MONSTER_BLACKBOARD&) const;
	virtual void				Decide_Internal(const MONSTER_BLACKBOARD&, _float) {}

protected:
	CMonster*					m_pOwner = { nullptr };
	_bool						m_bSpotted = { false };		// IDLE -> FIND 전이를 위한 변수

protected:
	virtual void				Free() override;
};

NS_END
