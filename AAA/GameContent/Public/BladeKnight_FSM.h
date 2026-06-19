#pragma once
#include "Monster_Brain_FSM_T.h"

NS_BEGIN(Client)
class CMonster;
class CBladeKnight;

class CBladeKnight_FSM final : public CMonster_Brain_FSM_T<CBladeKnight>
{
protected:
	CBladeKnight_FSM();
	virtual ~CBladeKnight_FSM() = default;

protected:
	HRESULT						Initialize();

protected:
	virtual void				On_Decide_Combat(CBladeKnight* pBladeKnight, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
	static CBladeKnight_FSM*	Create();

private:
	MONSTER_STATE_TYPE			Pick_AttackState(_int iAIType);

private:
	_uint						m_iAttackIndex = { 0 };

protected:
	virtual void				Free() override;
};

NS_END