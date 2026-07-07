#pragma once

#include "Kirby_ControllableState.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

enum JUMP_STATE_FLAG { NONE, FORCE_JUMP };

class CLIENT_DLL CKirby_Jump final : public CKirby_ControllableState
{
private:
	CKirby_Jump();
	virtual ~CKirby_Jump() = default;

	enum class JUMP_STATE { JUMP_START, JUMP, JUMP_END };

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
	JUMP_STATE m_eJumpType{};
	_bool m_bLeft{};

	_float m_fAccGroundIgnoreTime{};
	_float m_fMaxGroundIgnoreTime{};

public:
	static CKirby_Jump* Create();
private:
	virtual void Free() override;
};

NS_END