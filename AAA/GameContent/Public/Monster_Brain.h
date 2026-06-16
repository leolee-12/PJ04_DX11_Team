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
	virtual void	Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) = 0;

protected:
	virtual void	Free() override;
};

NS_END