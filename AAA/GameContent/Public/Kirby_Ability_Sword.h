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

class CLIENT_DLL CKirby_Ability_Sword final : public CKirby_Ability
{
private:
	enum SWORD_STATE
	{
		SLASH_1, SLASH_1_END, SLASH_2, SLASH_3,
		JUMP_SLASH_START, JUMP_SLASH,

		SPIN_SLASH_CHARGE,
		SPIN_SLASH, SPIN_SLASH_END,

		SUPER_SPIN_SLASH_CHARGE_START, SUPER_SPIN_SLASH_CHARGE,
		SUPER_SPIN_SLASH_START, SUPER_SPIN_SLASH_LOOP, SUPER_SPIN_SLASH_END,

		SWORD_STATE_END
	};

	enum SWORD_MOVE_STATE { NONE_MOVE, MOVE_FRONT, MOVE_RIGHT};

	enum SWORD_EFFECT
	{
		SLASH1, SLASH2_1, SLASH2_2, SLASH2_3, SLASH2_4, SLASH3, SPINSLASH, 
		JUMPSLASH, EFFECT_END
	};

private:
	CKirby_Ability_Sword();
	virtual ~CKirby_Ability_Sword() = default;

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
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

	virtual _bool Handle_BodyAnimEvent(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase) override;

public:
	virtual _bool Can_PlayJumpEndFromSlide() { return false; };

private:
	// State
	SWORD_STATE m_eSwordState{ SWORD_STATE::SWORD_STATE_END };
	SWORD_STATE m_eStartSwordState{ SWORD_STATE::SWORD_STATE_END };

	// Attack
	_bool m_bReserveNextAttack{};
	_bool m_bSpinSlashCharge{};
	_float m_fAccSuperSpinSlashChargeTime{};
	_uint m_iSuperSpinSlashCount{};

	// Move
	SWORD_MOVE_STATE m_eCurSwordMoveState{};
	SWORD_MOVE_STATE m_ePreSwordMoveState{};
	_float3 m_vSwordWishDir{};
	_bool m_bMoveLock{};

	// Effect
	_bool m_bIsStartEffect[SWORD_EFFECT::EFFECT_END]{};
	CEffect_Container* m_pSpinSlash{};
	CEffect_Container* m_pSpinSlashTrail{};
	CEffect_Container* m_pSwordChargeEffect{};
	CEffect_Container* m_pSwordSuperChargeEffect{};
	CEffect_Container* m_pUpwardSlash{};

private:
	void Change_SwordState(CKirby* pKirby, SWORD_STATE eNext);
	void Enter_SwordState(CKirby* pKirby, SWORD_STATE eState);
	void Update_SwordState(CKirby* pKirby, _float fTimeDelta);
	void Exit_SwordState(CKirby* pKirby, SWORD_STATE eState);

	void Update_SuperSpinSlashChargeTime(_float fTimeDelta);
	void Update_ChargeAnimationOverlay(CKirby* pKirby);

	void Update_MoveLockByRatio(_float fRatio, _float fRatioStart, _float fRatioEnd);
	void Update_MaxHorizontalSpeedByRatio(CMovement_Child* pMovement, _float fRatio, _float fRatioStart, _float fRatioEnd, _float fSpeed);

	_bool CanPlayEffect(SWORD_EFFECT eSwordEffect, CAnimator* pAnimator, _float fRatio);
	void FadeOut_SpinSlashEffect(CEffect_Container*& pEffectContainer, _float fFadeOutDuration);

public:
	static CKirby_Ability_Sword* Create();
private:
	virtual void Free() override;
};

NS_END
