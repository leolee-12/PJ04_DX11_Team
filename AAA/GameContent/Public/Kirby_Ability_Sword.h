#pragma once

#include "Kirby_Ability.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CMovement_Child;

class CLIENT_DLL CKirby_Ability_Sword final : public CKirby_Ability
{
private:
	enum SWORD_STATE { NONE, SLASH_1, SLASH_1_END, SLASH_2, SLASH_3 };

private:
	CKirby_Ability_Sword();
	virtual ~CKirby_Ability_Sword() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_Ability(CKirby* pKirby) override;
	virtual void Update_Ability(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_Ability(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual void Down_Attack(CKirby* pKirby) override;
	virtual void Up_Attack(CKirby* pKirby) override;

	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation) override;

private:
	SWORD_STATE m_eCurSwordState{};
	SWORD_STATE m_ePreSwordState{};

	_bool m_bReserveNextAttack{};

public:
	static CKirby_Ability_Sword* Create();
private:
	virtual void Free() override;
};

NS_END