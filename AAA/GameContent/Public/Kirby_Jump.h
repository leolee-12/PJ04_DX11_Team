#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Jump final : public CKirby_State
{
private:
	CKirby_Jump();
	virtual ~CKirby_Jump() = default;

	enum class JUMP_STATE
	{
		JUMP_STRAT, LAND_START, LAND_END,
	};

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
	_bool m_bFirstFrameSkip{};

	JUMP_STATE m_eJumpType{};

	static _bool m_bLeftRight;

public:
	static CKirby_Jump* Create();
private:
	virtual void Free() override;
};

NS_END