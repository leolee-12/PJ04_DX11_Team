#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_CarFirstBreakWall final : public CKirby_State
{
private:
	CKirby_CarFirstBreakWall();
	virtual ~CKirby_CarFirstBreakWall() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

public:
	virtual void Request_ReleaseGrabState(CKirby* pKirby, CUTSCENE_KIRBY_TYPE eType) override;

private:
	_bool m_bTurnStarted{};
	_float3 m_vLeftDir{};

public:
	static CKirby_CarFirstBreakWall* Create();
private:
	virtual void Free() override;
};

NS_END