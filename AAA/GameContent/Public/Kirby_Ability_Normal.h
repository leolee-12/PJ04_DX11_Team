#pragma once

#include "Kirby_Ability.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_Normal final : public CKirby_Ability
{
private:
	enum class INHALE_STATE
	{
		INHALE_LOOP,
		SUPER_INHALE_START, SUPER_INHALE_LOOP,
		INHALE_END
	};

	enum class INHALE_MOVE_STATE
	{
		WAIT, WALK, FALL
	};

private:
	CKirby_Ability_Normal();
	virtual ~CKirby_Ability_Normal() = default;

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
	INHALE_STATE m_eInhaleState{};

	INHALE_MOVE_STATE m_eCurMoveState{};
	INHALE_MOVE_STATE m_ePreMoveState{};

	_float m_MaxSuperInHaleTime{};
	_float m_AccSuperInHaleTime{};

	_bool m_bForceEnterSuperInhaleStart{};

private:
	void Interpolation_Inhale(CAnimator* pAnimator);
	void Choose_InhaleAniName(_string& strAniName);

public:
	static CKirby_Ability_Normal* Create();
private:
	virtual void Free() override;
};

NS_END