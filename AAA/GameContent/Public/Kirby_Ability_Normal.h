#pragma once

#include "Kirby_Ability.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CKirby;
class CMovement_Child;

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
	virtual ABILITY_UPDATE_RESULT Update_Ability(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_Ability(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation) override;

private:
	INHALE_STATE m_eInhaleState{};

	INHALE_MOVE_STATE m_eCurMoveState{};
	INHALE_MOVE_STATE m_ePreMoveState{};

	_float m_MaxSuperInHaleTime{};
	_float m_AccSuperInHaleTime{};

	_bool m_bForceEnterSuperInhaleStart{};

	_bool m_bReqInhale{};

private:
	void Interpolation_Inhale(CAnimator* pAnimator);
	void Choose_InhaleAniName(_string& strAniName);

	_bool Change_Ability(CKirby* pKirby);

	void Reset_Default(CKirby* pKirby);

	CEffect_Container* m_pInhaleEffect{};

public:
	static CKirby_Ability_Normal* Create();
private:
	virtual void Free() override;
};

NS_END