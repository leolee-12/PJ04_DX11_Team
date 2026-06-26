#pragma once

#include "Base.h"
#include "GameContent_Defines.h"
#include "Monster_BlackBoard.h"

NS_BEGIN(Client)

class CMonster;

class CMonsterBrain abstract : public CBase
{
protected:
	CMonsterBrain();
	virtual ~CMonsterBrain() = default;

public:
	virtual void	Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) = 0;

protected:
	CMonster*		m_pOwner = { nullptr };

protected:
	virtual void	Free() override;
};

NS_END