#pragma once

#include "Kirby_ControllableState.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Guard final : public CKirby_ControllableState
{
private:
	static constexpr _float s_fGuardGroundFriction = 10.f;

private:
	CKirby_Guard();
	virtual ~CKirby_Guard() = default;

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

public:
	virtual void  On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

private:
	CGameInstance_Proxy* m_pGameInstance_Proxy{};

public:
	static CKirby_Guard* Create();
private:
	virtual void Free() override;
};

NS_END