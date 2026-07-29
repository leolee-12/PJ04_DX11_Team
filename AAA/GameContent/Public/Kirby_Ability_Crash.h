#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_Crash final : public CKirby_Ability
{
private:
	enum class CRASH_STATE
	{
		FLAME_CHARGE_START, FLAME_CHARGE,
		FLAME_START, FLAME, DAMAGE, FLAME_END,
		CRASH_STATE_END
	};

	enum class CRASH_DAMAGE_MODE { DEFAULT, JUMP };

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

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

public:
	virtual _bool Ignore_TimeScale() override { return true; }

private:
	void Change_CrashState(CKirby* pKirby, CRASH_STATE eNext);
	void Enter_CrashState(CKirby* pKirby, CRASH_STATE eState);
	void Update_CrashState(CKirby* pKirby, _float fTimeDelta);
	void Exit_CrashState(CKirby* pKirby, CRASH_STATE eState);

private:
	CRASH_STATE m_eCrashState{ CRASH_STATE::CRASH_STATE_END };

	_float m_fAccFlameChargeTime{};
	_float m_fAccFlameTime{};
	_float m_fAccDamageTime{};
	_uint m_iAccDamageRotCount{};

	_bool m_bKeyUpAttackEnd{};

	CRASH_DAMAGE_MODE m_eCrashDamageMode{};

	_float3 m_vDamageStartPos{};

public:
	static CKirby_Ability_Crash* Create();
private:
	virtual void Free() override;
};

NS_END
