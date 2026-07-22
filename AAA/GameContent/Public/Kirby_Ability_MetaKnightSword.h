#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CLIENT_DLL CKirby_Ability_MetaKnightSword final : public CKirby_Ability
{
private:
	CKirby_Ability_MetaKnightSword();
	virtual ~CKirby_Ability_MetaKnightSword() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

public:
	static CKirby_Ability_MetaKnightSword* Create();

private:
	virtual void Free() override;
};

NS_END