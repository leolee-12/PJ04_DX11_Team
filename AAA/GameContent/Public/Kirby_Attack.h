#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Attack final : public CKirby_State
{
private:
	CKirby_Attack();
	virtual ~CKirby_Attack() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

public:
	virtual void  On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

private:
	_bool m_bGuardReserved{};

public:
	static CKirby_Attack* Create();
private:
	virtual void Free() override;
};

NS_END