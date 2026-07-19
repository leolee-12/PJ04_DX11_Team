#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_Sleep final : public CKirby_Ability
{
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

public:
	static CKirby_Ability_Sleep* Create();

private:
	virtual void Free() override;
};

NS_END
