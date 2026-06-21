#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)
class CMonster;
class CBladeKnight;

class CBladeKnight_FSM final : public CMonster_Brain_FSM
{
protected:
	CBladeKnight_FSM();
	virtual ~CBladeKnight_FSM() = default;

protected:
	virtual HRESULT				Initialize(CMonster* pOwner) override;

protected:
	virtual void				Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
	static CBladeKnight_FSM*	Create(CMonster* pMonster);

private:
	MONSTER_STATE_TYPE			Pick_AttackState(_int iAIType);

private:
	_uint						m_iAttackIndex = { 0 };

protected:
	virtual void				Free() override;
};

NS_END