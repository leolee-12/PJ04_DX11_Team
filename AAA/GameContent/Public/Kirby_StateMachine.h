#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_State;

enum class KIRBY_STATE_TYPE;

struct ATTACK_INFO;

class CLIENT_DLL CKirby_StateMachine final : public CBase
{
private:
	CKirby_StateMachine();
	virtual ~CKirby_StateMachine() = default;

private:
	HRESULT Initialize(CKirby* pKirby);

public:
	KIRBY_STATE_TYPE Get_StateType();

public:
	void Change_State(KIRBY_STATE_TYPE eNewstate, _int iFlag = -1);
	void Update_StateMachine(const _float fTimeDelta);
	void Handle_Command(CKirby_Command* pCommand);

public:
	void On_KirbyCollisionEnter_StateMachine(_uint iColliderType, CCollider* pOther);
	void On_KirbyCollisionStay_StateMachine(_uint iColliderType, CCollider* pOther);
	void On_KirbyCollisionExit_StateMachine(_uint iColliderType, CCollider* pOther);

public:
	void  On_Damaged_KirbyStateMachine(const ATTACK_INFO& tInfo);

public:
	// Attach
	void Request_Attachment_StateMachine(const KIRBY_ATTACHMENT_BEGIN_DESC* pDesc);
	void Request_Attachment_End_StateMachine(const KIRBY_ATTACHMENT_END_DESC* pDesc);

	// Position
	void Request_PositionSync_StateMachine(const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc);
	void Request_PositionSync_End_StateMachine(const KIRBY_POSITION_SYNC_END_DESC* pDesc);

	void Request_ClearStage_StateMachine(const CUTSCENE_STAGECLEAR* pDesc);

	// Dialogue
	void Request_Dialogue_StateMachine(const SEQUENCE_KIRBY_WARP_DESC* pDesc);
	void Request_DialogueAnim_StateMachine(const SEQUENCE_KIRBY_ANIM_DESC* pDesc);

	// SequenceLock
	void Request_SequenceLock_StateMachine(const KIRBY_LEVEL_SLEEP_DESC* pDesc);
	void Request_SequenceLock_End_StateMachine(const KIRBY_LEVEL_SPAWN_DESC* pDesc);

public:
	void Get_EssenceBubble(COPY_ABILITY_TYPE eNewAbility);

	_bool Ignore_TimeScale_StateMachine();

private:
	CKirby* m_pKirby{};
	CKirby_State* m_pCurState{};

	unordered_map<KIRBY_STATE_TYPE, CKirby_State*> m_States;

	CGameInstance_Proxy* m_pGameInstance_Proxy{};
private:
	HRESULT Init_State();

	CKirby_State* Find_State(KIRBY_STATE_TYPE eNewstate);

public:
	static CKirby_StateMachine* Create(CKirby* pKirby);
private:
	virtual void Free() override;
};

NS_END