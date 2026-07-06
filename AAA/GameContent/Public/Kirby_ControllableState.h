#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_ControllableState abstract : public CKirby_State
{
protected:
	CKirby_ControllableState();
	virtual ~CKirby_ControllableState() = default;

protected:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() = 0;

public:
	virtual void Enter(CKirby* pKirby);
	virtual void Update(CKirby* pKirby, const _float fTimeDelta);
	virtual void Exit(CKirby* pKirby);

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand);

protected:
	virtual void Free() override;
};

NS_END