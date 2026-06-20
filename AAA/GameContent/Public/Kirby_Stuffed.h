#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_Stuffed final : public CKirby_State
{
private:
	CKirby_Stuffed();
	virtual ~CKirby_Stuffed() = default;

	enum STUFFED_STATE { STUFFED_START, STUFFED_WAIT, STUFFED_END };

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

private:
	void Update_State(CKirby* pKirby);
	void Enter_Animation(CKirby* pKirby, CAnimator* pAnimator);

private:
	STUFFED_STATE m_eCurStuffedState{};

public:
	static CKirby_Stuffed* Create();
private:
	virtual void Free() override;
};

NS_END