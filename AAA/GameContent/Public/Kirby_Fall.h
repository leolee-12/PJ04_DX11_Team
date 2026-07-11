#pragma once

#include "Kirby_ControllableState.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

enum FALL_STATE_FLAG { FALL_DIRECT, PLAY_JUMP_END_L, PLAY_JUMP_END_R};

class CLIENT_DLL CKirby_Fall final : public CKirby_ControllableState
{
private:
	CKirby_Fall();
	virtual ~CKirby_Fall() = default;

	enum class FALL_STATE { JUMP_END, FALLING, LAND_START, FALL_STATE_END };

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
	void Change_FallState(CKirby* pKirby, FALL_STATE eNext);
	void Enter_FallState(CKirby* pKirby, FALL_STATE eState);
	void Update_FallState(CKirby* pKirby, _float fTimeDelta);
	void Exit_FallState(CKirby* pKirby, FALL_STATE eState);

private:
	FALL_STATE m_eFallState{};

	_bool m_bGuardReserved{};

	_bool m_bLeft{};

public:
	static CKirby_Fall* Create();
private:
	virtual void Free() override;
};

NS_END