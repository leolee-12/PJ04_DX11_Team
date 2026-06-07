#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

enum class KIRBY_STATE_TYPE
{
	WAIT, RUN, JUMP, 
	ATTACK,
};

class CLIENT_DLL CKirby_State abstract : public CBase
{
protected:
	CKirby_State();
	virtual ~CKirby_State() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() = 0;

public:
	virtual void Enter(CKirby* pKirby);
	virtual void Update(CKirby* pKirby, const _float fTimeDelta);
	virtual void End(CKirby* pKirby);

public:
	virtual _bool Handle_Command(CKirby* pKirby, const CKirby_Command& Command);

protected:
	virtual void Free() override;
};

NS_END