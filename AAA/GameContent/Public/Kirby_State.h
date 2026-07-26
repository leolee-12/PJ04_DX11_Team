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
	EMOTE,

	CUTSCENE_GRABBED, CAR_FIRST_BREAK_WALL, DEFORM_CAR_BRIDGE,
	METAKNIGHT_ENCOUNTER, METAKNIGHT_QTE,
	QTE_GRABBED,
	STAGE_CLEAR,
	SEQUENCE_LOCK,
	DIALOGUE
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

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand);

public:
	virtual void On_KirbyCollisionEnter(CKirby* pKirby, _uint iColliderType, CCollider* pOther);
	virtual void On_KirbyCollisionStay(CKirby* pKirby, _uint iColliderType, CCollider* pOther);
	virtual void On_KirbyCollisionExit(CKirby* pKirby, _uint iColliderType, CCollider* pOther);

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo);

public:
	virtual _bool Ignore_TimeScale() { return false; }

	virtual void Request_Attachment(CKirby* pKirby, const KIRBY_ATTACHMENT_BEGIN_DESC* pDesc) {}
	virtual void Request_Attachment_End(CKirby* pKirby, const KIRBY_ATTACHMENT_END_DESC* pDesc) {}

	virtual void Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc) {}
	virtual void Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc) {}

	virtual void Request_StageClear(CKirby* pKirby, const CUTSCENE_STAGECLEAR* pDesc) {}

	// Dialogue
	virtual void Request_Dialogue(CKirby* pKirby, const SEQUENCE_KIRBY_WARP_DESC* pDesc) {}
	virtual void Request_DialogueAnim(CKirby* pKirby, const SEQUENCE_KIRBY_ANIM_DESC* pDesc) {}

	// SequenceLock
	virtual void Request_SequenceLock(CKirby* pKirby, const KIRBY_LEVEL_SLEEP_DESC* pDesc) {}
	virtual void Request_SequenceLock_End(CKirby* pKirby, const KIRBY_LEVEL_SPAWN_DESC* pDesc) {}

	virtual void Cleanup_ForLevelTransition(CKirby* pKirby, const KIRBY_LEVEL_SLEEP_DESC* pDesc) {}

	// MetaKnight
	virtual void Request_MetaKnight_ParryBegin(CKirby* pKirby) {};

protected:
	_bool Handle_MoveCommand(CKirby* pKirby, CKirby_Command* pCommand);

protected:
	_bool Try_Transition_Fall(CKirby* pKirby);
	_bool Try_Transition_Fall_Immediate(CKirby* pKirby);
	_bool Transition_Wait_OR_Run(CKirby* pKirby);
	_bool Transition_Fall_OR_Wait_OR_Run(CKirby* pKirby);
	_bool Transition_Fall_OR_Wait_OR_Run_Immediate(CKirby* pKirby);
	_bool Try_Transition_Ladder_CommandUp(CKirby* pKirby);
	_bool Try_Transition_Ladder_CommandDown(CKirby* pKirby);

	void Effect_Stop(CEffect_Container*& pContainer);

protected:
	CGameInstance_Proxy* m_pGameInstance_Proxy{};

protected:
	virtual void Free() override;
};

NS_END