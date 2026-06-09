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
	void Change_State(KIRBY_STATE_TYPE eNewstate);
	void Update_StateMachine(const _float fTimeDelta);
	void Handle_Command(CKirby_Command* pCommand);

private:
	CKirby* m_pKirby{};
	CKirby_State* m_pCurState{};

private:
	CKirby_State* State_Creator(KIRBY_STATE_TYPE eNewstate);

public:
	static CKirby_StateMachine* Create(CKirby* pKirby);
private:
	virtual void Free() override;
};

NS_END