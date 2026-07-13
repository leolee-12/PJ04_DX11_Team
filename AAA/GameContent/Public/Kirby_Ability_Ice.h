#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_Ice final : public CKirby_Ability
{
private:
	CKirby_Ability_Ice();
	virtual ~CKirby_Ability_Ice() = default;

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

public:
	static CKirby_Ability_Ice* Create();

private:
	virtual void Free() override;
};

NS_END
