#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)
class CMonster;

// State ~ CMonster_StateT와 동일 구조 : cast를 여기서 1회
// 파생 Brain들은 타입(T*)이 박힌 On_Decde_Combat만 구현
template<class T>
class CMonster_Brain_FSM_T : public CMonster_Brain_FSM
{
protected:
	CMonster_Brain_FSM_T() = default;
	virtual ~CMonster_Brain_FSM_T() = default;

protected:
	virtual void	On_Decide_Combat(T*, const MONSTER_BLACKBOARD&, _float) {};

private:
	virtual void	Decide_Combat(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) final
	{
		On_Decide_Combat(static_cast<T*>(pMonster), BlackBoard, fTimeDelta);
	}
};

NS_END