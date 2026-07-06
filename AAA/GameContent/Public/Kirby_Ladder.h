#pragma once

#include "Kirby_ControllableState.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ladder final : public CKirby_ControllableState
{
private:
	static constexpr _float s_fLadderSpeed = 8.f;

	enum LADDER_STATE { WAIT, MOVE };

private:
	CKirby_Ladder();
	virtual ~CKirby_Ladder() = default;

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
	LADDER_STATE m_eLadderState{};

	_int m_iCurLadderIndex{};
	_int m_iNextLadderIndex{};

private:
	void Change_LadderState(CKirby* pKirby, LADDER_STATE eNext);
	void Enter_LadderState(CKirby* pKirby, LADDER_STATE eState);
	void Update_LadderState(CKirby* pKirby, _float fTimeDelta);
	void Exit_LadderState(CKirby* pKirby, LADDER_STATE eState);

public:
	static CKirby_Ladder* Create();
private:
	virtual void Free() override;
};

NS_END