#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
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
	void  On_Damaged_KirbyStateMachine(const ATTACK_INFO& tInfo);

public:
	void Request_GrabState_StateMachine(KIRBY_ATTACHMENT_CONTEXT eType);
	void Request_ReleaseGrabState_StateMachine(KIRBY_ATTACHMENT_END_REASON eType = KIRBY_ATTACHMENT_END_REASON::DEFAULT_RELEASE);
	void Request_ClearStage_StateMachine(const CUTSCENE_STAGECLEAR* pDesc);

	void Get_EssenceBubble(COPY_ABILITY_TYPE eNewAbility);

	_bool Ignore_TimeScale_StateMachine();

private:
	CKirby* m_pKirby{};
	CKirby_State* m_pCurState{};

	unordered_map<KIRBY_STATE_TYPE, CKirby_State*> m_States;

private:
	HRESULT Init_State();

	CKirby_State* Find_State(KIRBY_STATE_TYPE eNewstate);

public:
	static CKirby_StateMachine* Create(CKirby* pKirby);
private:
	virtual void Free() override;
};

NS_END