#pragma once

#include "Base.h"
#include "GameContent_Defines.h"
#include "Monster_BlackBoard.h"

NS_BEGIN(Client)

class CMonster;

class CLIENT_DLL CMonster_State abstract : public CBase
{
protected:
	CMonster_State();
	virtual ~CMonster_State() = default;

public:
	virtual MONSTER_STATE_TYPE	Get_StateType() = 0;

public:
	virtual void				Enter(CMonster* pMonster);
	virtual void				Update(CMonster* pMonster, _float fTimeDelta);
	virtual void				Exit(CMonster* pMonster);

protected:
	virtual void				Free() override;
};

NS_END
