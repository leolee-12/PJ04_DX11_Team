#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CMovement_Child;

class CLIENT_DLL CKirby_Hovering final : public CKirby_State
{
private:
	CKirby_Hovering();
	virtual ~CKirby_Hovering() = default;

	enum HOVERING_STATE { FLIGHT_START, FLIGHT_LOOP, FLIGHT_END, SPIT_AIR };
	// Loop에서 두 상태로 분기
	enum HOVERING_MOVE_STATE { FALL, JUMP};

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

	_bool m_bPlayFlightAni{};

private:
	_bool Update_HoveringStateMachine(CKirby* pKirby, _float fTimeDelta);
	_bool Check_Landing(CKirby* pKirby, CAnimator *pAnimator, CMovement_Child* pMovement);

	void Update_LoopState(CKirby* pKirby, _float fTimeDelta);

public:
	static CKirby_Hovering* Create();
private:
	virtual void Free() override;
};

NS_END