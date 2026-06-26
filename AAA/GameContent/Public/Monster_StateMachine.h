#pragma once

#include "Base.h"
#include "GameContent_Defines.h"
#include "Monster_BlackBoard.h"

NS_BEGIN(Client)

class CMonster;
class CMonster_State;

class CMonster_StateMachine final : public CBase
{
private:
	CMonster_StateMachine();
	virtual ~CMonster_StateMachine() = default;

private:
	HRESULT							Initialize(CMonster* pMonster);

public:
	MONSTER_STATE_TYPE				Get_StateType();

public:
	void							Update_StateMachine(_float fTimeDelta);

	_bool							Change_State(MONSTER_STATE_TYPE eNewState);
	HRESULT							Register_State(MONSTER_STATE_TYPE eType, CMonster_State* pState);
	_bool							Has_State(MONSTER_STATE_TYPE eType) const;

private:
	CMonster*						m_pOwner = { nullptr };
	CMonster_State*					m_pCurState = { nullptr };

	MONSTER_STATE_TYPE				m_PrevState = { MONSTER_STATE_TYPE::IDLE }; // Enter 때 이전 상태가 어떤 타입이었는지 던져줌 

	unordered_map<MONSTER_STATE_TYPE, CMonster_State*> m_States;

private:
	CMonster_State*					Find_State(MONSTER_STATE_TYPE eNewState);

public:
	static CMonster_StateMachine*	Create(CMonster* pOwner);

protected:
	virtual void					Free() override;
};

NS_END