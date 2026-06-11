#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Hovering final : public CKirby_State
{
private:
	CKirby_Hovering();
	virtual ~CKirby_Hovering() = default;

	enum HOVERING_STATE { FLIGHT_START, FLIGHT_LOOP, FLIGHT_END, SPITAIR };
	enum HOVERING_MOVE_STATE { FALL, MOVE };

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby) override;
	virtual void Update(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

private:
	HOVERING_STATE m_eHoveringState{};

	HOVERING_MOVE_STATE m_eCurMoveState{};
	HOVERING_MOVE_STATE m_ePreMoveState{};

	_float m_fMaxJumpTime{};
	_float m_fAccJumpTime{};
	_bool m_bCanJump{};

private:
	_bool Update_State(CKirby* pKirby, _float fTimeDelta);

	void Enter_Ani(CKirby* pKirby);
	void Update_MoveState(CKirby* pKirby, _float fTimeDelta);

	void Update_CoolTimer(_float fTimeDelta);
	void Reset_CoolTimer();

public:
	static CKirby_Hovering* Create();
private:
	virtual void Free() override;
};

NS_END