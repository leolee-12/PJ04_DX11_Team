#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_GetAbility final : public CKirby_State
{
private:
	CKirby_GetAbility();
	virtual ~CKirby_GetAbility() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

public:
	virtual _bool Ignore_TimeScale() { return true; }

private:
	_bool m_bPartsOn{};
	_bool m_bCloseEye{};
	_bool m_bOpenMouse{};

private:
	void Parts_On(CKirby* pKirby, _float fRatio);
	void Close_Eye(CKirby_Body* pBody, _float fRatio);
	void Open_Mouse(CKirby_Body* pBody, _float fRatio);

public:
	static CKirby_GetAbility* Create();
private:
	virtual void Free() override;
};

NS_END