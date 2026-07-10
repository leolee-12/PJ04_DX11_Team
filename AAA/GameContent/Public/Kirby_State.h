#pragma once

#include "Base.h"

#include "GameContent_const.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
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
	SLIDE,
	DODGE,
	LADDER,
	GET_DEFORM, DEFORM_DUMP,
	CUTSCENE_GRABBED, QTE_GRABBED, CAR_FIRST_BREAK_WALL, DEFORM_CAR_BRIDGE,
	STAGE_CLEAR
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
	virtual void Enter(CKirby* pKirby, _int iFlag);
	virtual void Update(CKirby* pKirby, const _float fTimeDelta);
	virtual void Exit(CKirby* pKirby);

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo);

protected:
	_bool Handle_MoveCommand(CKirby* pKirby, CKirby_Command* pCommand);

protected:
	_bool Try_Transition_Fall(CKirby* pKirby);
	_bool Transition_Wait_OR_Run(CKirby* pKirby);
	_bool Transition_Fall_OR_Wait_OR_Run(CKirby* pKirby);
	_bool Try_Transition_Ladder_CommandUp(CKirby* pKirby);
	_bool Try_Transition_Ladder_CommandDown(CKirby* pKirby);

protected:
	CGameInstance_Proxy* m_pGameInstance_Proxy{};

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand);

public:
	virtual void Request_Attachment(CKirby* pKirby, const KIRBY_ATTACHMENT_BEGIN_DESC* pDesc);
	virtual void Request_Attachment_End(CKirby* pKirby, const KIRBY_ATTACHMENT_END_DESC* pDesc);

	virtual void Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc);
	virtual void Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc);

	virtual void Request_StageClear(CKirby* pKirby, const CUTSCENE_STAGECLEAR* pDesc);

	virtual _bool Ignore_TimeScale() { return false; }

protected:
	virtual void Free() override;
};

NS_END