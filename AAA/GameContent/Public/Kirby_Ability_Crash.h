#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_Crash final : public CKirby_Ability
{
private:
	enum class CRASH_STATE { FLAME_CHARGE_START, FLAME_CHARGE, FLAME_START, FLAME, FLAME_END, CRASH_STATE_END };

private:
	CKirby_Ability_Crash();
	virtual ~CKirby_Ability_Crash() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

private:
	void Change_CrashState(CKirby* pKirby, CRASH_STATE eNext);
	void Enter_CrashState(CKirby* pKirby, CRASH_STATE eState);
	void Update_CrashState(CKirby* pKirby, _float fTimeDelta);
	void Exit_CrashState(CKirby* pKirby, CRASH_STATE eState);

	CRASH_STATE m_eCrashState{};

public:
	static CKirby_Ability_Crash* Create();
private:
	virtual void Free() override;
};

NS_END
