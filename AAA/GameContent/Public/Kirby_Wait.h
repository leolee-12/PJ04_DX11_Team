#pragma once

#include "Kirby_ControllableState.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Wait final : public CKirby_ControllableState
{
private:
	CKirby_Wait();
	virtual ~CKirby_Wait() = default;

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

public:
	static CKirby_Wait* Create();
private:
	virtual void Free() override;
};

NS_END