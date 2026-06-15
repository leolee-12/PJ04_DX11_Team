#pragma once

#include "Base.h"
#include "GameContent_Defines.h"
#include "Monster_BlackBoard.h"

NS_BEGIN(Client)

class CMonster;
class CMonster_State;

class CLIENT_DLL CMonster_StateMachine final : public CBase
{
private:
	CMonster_StateMachine();
	virtual ~CMonster_StateMachine() = default;

private:
	HRESULT							Initialize(CMonster* pMonster);

public:
	MONSTER_STATE_TYPE				Get_StateType();

public:
	void							Change_State(MONSTER_STATE_TYPE eNewState);
	void							Update_StateMachine(_float fTimeDelta);

private:
	CMonster*						m_pMonster = { nullptr };
	CMonster_State*					m_pCurState = { nullptr };

private:
	CMonster_State*					State_Creator(MONSTER_STATE_TYPE eNewState);

public:
	static CMonster_StateMachine*	Create(CMonster* pMonster);

protected:
	virtual void					Free() override;
};

NS_END