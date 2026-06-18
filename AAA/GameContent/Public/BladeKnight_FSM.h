#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)
class CMonster;

class CBladeKnight_FSM final : public CMonster_Brain_FSM
{
protected:
	CBladeKnight_FSM();
	virtual ~CBladeKnight_FSM() = default;

protected:
	HRESULT						Initialize();

public:
	virtual void				Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
	static CBladeKnight_FSM*	Create();

protected:
	virtual void				Free() override;
};

NS_END