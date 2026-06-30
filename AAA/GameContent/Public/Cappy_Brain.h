#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)
class CMonster;

class CCappy_Brain final : public CMonster_Brain_FSM
{
protected:
	CCappy_Brain() = default;
	virtual ~CCappy_Brain() = default;

protected:
	virtual HRESULT			Initialize(CMonster* pOwner) override;

public:
	static CCappy_Brain*	Create(CMonster* pOwner);

protected:
	virtual void			Free() override;
};

NS_END