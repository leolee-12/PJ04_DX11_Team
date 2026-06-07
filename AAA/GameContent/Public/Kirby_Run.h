#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Run final : public CKirby_State
{
protected:
	CKirby_Run();
	virtual ~CKirby_Run() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void End(CKirby* pKirby) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, const CKirby_Command& Command) override;

public:
	static CKirby_Run* Create();
private:
	virtual void Free() override;
};

NS_END