#pragma once

#include "Base.h"

#include "GameContent_const.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CKirby;
struct ATTACK_INFO;

enum class KIRBY_ATTACK_LOCATION { GROUND, AIR };

class CLIENT_DLL CKirby_AttackMode abstract : public CBase
{
protected:
	CKirby_AttackMode();
	virtual ~CKirby_AttackMode() = default;

protected:
	HRESULT Initialize();

public:
	virtual void Enter_AttackState(CKirby* pKirby) = 0;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) = 0;
	virtual void Exit_AttackState(CKirby* pKirby) = 0;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo);

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) = 0;

public:
	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) = 0;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) = 0;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) = 0;

public:
	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation);

	_bool ReqEndAttackState() { return m_bReqEndAttackState; }

protected:
	void Effect_Stop(CEffect_Container*& pContainer1);

protected:
	_bool m_bReqEndAttackState{ true };

	CGameInstance_Proxy* m_pGameInstance_Proxy{};

protected:
	virtual void Free() override;
};

NS_END