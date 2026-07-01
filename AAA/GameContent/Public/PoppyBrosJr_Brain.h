#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)
class CMonster;
class CPoppyBrosJr;

class CPoppyBrosJr_Brain final : public CMonster_Brain_FSM
{
protected:
	CPoppyBrosJr_Brain() = default;
	virtual ~CPoppyBrosJr_Brain() = default;

protected:
	virtual HRESULT				Initialize(CMonster* pOwner) override;
	virtual void				Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;
	virtual void				Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
	static CPoppyBrosJr_Brain*	Create(CMonster* pOwner);

protected:
	virtual void				Free() override;
};

NS_END