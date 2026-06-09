#pragma once

#include "Kirby_Ability.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_Normal final : public CKirby_Ability
{
private:
	CKirby_Ability_Normal();
	virtual ~CKirby_Ability_Normal() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Down_Attack(CKirby* pKirby) override;
	virtual void Up_Attack(CKirby* pKirby) override;

public:
	static CKirby_Ability_Normal* Create();
private:
	virtual void Free() override;
};

NS_END