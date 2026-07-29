#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_Sleep final : public CKirby_Ability
{
private:
	enum SLEEP_STATE
	{
		SLEEP_WAIT_START,
		SLEEP_MOVE_ACTIVE,
		SLEEP_START,
		SLEEP,
		SLEEP_END_WAKE_UP,
		SLEEP_STATE_END
	};

	enum SLEEP_BEFORE_STATE
	{
		SLEEP_WAIT,
		SLEEP_WALK,
		SLEEP_FALL,
		SLEEP_BEFORE_STATE_END
	};

private:
	CKirby_Ability_Sleep();
	virtual ~CKirby_Ability_Sleep() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

public:
	virtual _bool Should_ForceEnterAttackState() override { return true; }

private:
	void Change_SleepState(CKirby* pKirby, SLEEP_STATE eNext);
	void Enter_SleepState(CKirby* pKirby, SLEEP_STATE eState);
	void Update_SleepState(CKirby* pKirby, _float fTimeDelta);
	void Exit_SleepState(CKirby* pKirby, SLEEP_STATE eState);

	void Change_SleepBeforeState(CKirby* pKirby, SLEEP_BEFORE_STATE eNext);
	void Enter_SleepBeforeState(CKirby* pKirby, SLEEP_BEFORE_STATE eState);
	void Update_SleepBeforeState(CKirby* pKirby, _float fTimeDelta);
	void Exit_SleepBeforeState(CKirby* pKirby, SLEEP_BEFORE_STATE eState);

	_bool Has_MoveDir();

	_bool SleepBeforeTimer(_float fTimeDelta);

private:
	SLEEP_STATE m_eSleepState{ SLEEP_STATE_END };
	SLEEP_BEFORE_STATE m_eSleepBeforeState{ SLEEP_BEFORE_STATE_END };
	
	_float3 m_vMoveDir{};
	_float m_fAccSleepBeforeTime{};
	_uint m_iSleepAniCount{};

public:
	static CKirby_Ability_Sleep* Create();
private:
	virtual void Free() override;
};

NS_END
