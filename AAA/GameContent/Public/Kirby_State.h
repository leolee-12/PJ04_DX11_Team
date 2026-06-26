#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

struct ATTACK_INFO;

enum class KIRBY_STATE_TYPE
{
	WAIT, RUN, JUMP, FALL,
	ATTACK,
	HOVERING,
	GET_ABILITY, ABILITY_DUMP,
	DAMAGED,
	GUARD,
	CUTSCENE_GRABBED, QTE_GRABBED,
};

class CLIENT_DLL CKirby_State abstract : public CBase
{
protected:
	CKirby_State();
	virtual ~CKirby_State() = default;

protected:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() = 0;

public:
	virtual void Enter(CKirby* pKirby);
	virtual void Update(CKirby* pKirby, const _float fTimeDelta);
	virtual void Exit(CKirby* pKirby);

public:
	virtual void  On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo);

protected:
	_bool Handle_MoveCommand(CKirby* pKirby, CKirby_Command* pCommand);

protected:
	_bool Try_Transition_Fall(CKirby* pKirby);
	_bool Transition_Wait_OR_Run(CKirby* pKirby);
	_bool Transition_Fall_OR_Wait_OR_Run(CKirby* pKirby);

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand);

public:
	virtual void Request_GrabState(CKirby* pKirby, GRAB_TYPE eType);
	virtual void Request_ReleaseGrabState(CKirby* pKirby, GRAB_TYPE eType);

protected:
	virtual void Free() override;
};

NS_END