#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)
class CMonster;

class CNormalEnemy_Brain final : public CMonster_Brain_FSM
{
protected:
	CNormalEnemy_Brain();
	virtual ~CNormalEnemy_Brain() = default;

protected:
	virtual HRESULT				Initialize(CMonster* pOwner) override;

protected:
	virtual void				Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
	static CNormalEnemy_Brain*	Create(CMonster* pOwner);

private:
	static constexpr _float		TRANSITION_TIME = 2.5f;

protected:
	virtual void				Free() override;

};

NS_END