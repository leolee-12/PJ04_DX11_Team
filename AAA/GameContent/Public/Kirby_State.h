#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

enum class KIRBY_STATE_TYPE
{
	WAIT, RUN, JUMP, FALL,
	ATTACK,
	HOVERING,
	GET_ABILITY, ABILITY_DUMP
};

class CLIENT_DLL CKirby_State abstract : public CBase
{
protected:
	CKirby_State();
	virtual ~CKirby_State() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() = 0;

public:
	virtual void Enter(CKirby* pKirby);
	virtual void Update(CKirby* pKirby, const _float fTimeDelta);
	virtual void Exit(CKirby* pKirby);

protected:
	_bool Handle_MoveCommand(CKirby* pKirby, CKirby_Command* pCommand);

protected:
	_bool Try_FallState(CKirby* pKirby);
	_bool Transition_Wait_OR_Run(CKirby* pKirby);

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand);



protected:
	virtual void Free() override;
};

NS_END