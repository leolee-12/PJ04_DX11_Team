#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_Clear final : public CKirby_State
{
private:
	enum CLEAR_STATE { CUT1, DANCE, CLEAR_END };

private:
	CKirby_Clear();
	virtual ~CKirby_Clear() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby, _int iFlag) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

private:
	void Change_ClearState(CKirby* pKirby, CLEAR_STATE eNext);
	void Enter_ClearState(CKirby* pKirby, CLEAR_STATE eState);
	void Update_ClearState(CKirby* pKirby, _float fTimeDelta);
	void Exit_ClearState(CKirby* pKirby, CLEAR_STATE eState);
	
private:
	CLEAR_STATE m_eClearState{};

public:
	static CKirby_Clear* Create();
private:
	virtual void Free() override;
};

NS_END