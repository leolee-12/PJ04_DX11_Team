#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Fall final : public CKirby_State
{
private:
	CKirby_Fall();
	virtual ~CKirby_Fall() = default;

	enum class FALL_STATE { FALLING, LAND_START };

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
	FALL_STATE m_eFallingState{};

public:
	static CKirby_Fall* Create();
private:
	virtual void Free() override;
};

NS_END