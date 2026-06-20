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
	enum SWORD_STATE
	{
		NONE,
		SLASH_1, SLASH_1_END, SLASH_2, SLASH_3,
		JUMP_SLASH_START, JUMP_SLASH,

		SPIN_SLASH_CHARGE,
		SPIN_SLASH, SPIN_SLASH_END,

		SUPER_SPIN_SLASH_CHARGE_START, SUPER_SPIN_SLASH_CHARGE,
		SUPER_SPIN_SLASH_START, SUPER_SPIN_SLASH_LOOP, SUPER_SPIN_SLASH_END,
	};

private:
	CKirby_Ability_Sword();
	virtual ~CKirby_Ability_Sword() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_Ability(CKirby* pKirby) override;
	virtual ABILITY_UPDATE_RESULT Update_Ability(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_Ability(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation) override;

private:
	SWORD_STATE m_eCurSwordState{};
	SWORD_STATE m_ePreSwordState{};

	_bool m_bReserveNextAttack{};

	// Charge	
	_bool m_bSpinSlashCharge{};

	_float m_fAccSuperSpinSlashChargeTime{};
	_float m_fSuperSpinSlashChargeTime{};

	_uint m_iSuperSpinSlashCount{};
	_bool m_bForceEnterSwordAni{};

	// Dir
	_float3 m_vSwordWishDir{};
	_bool m_bMoveLock{};

private:
	void Update_SwordState(CKirby* pKirby, CAnimator* pAnimator, CMovement_Child* pMovemet, _float fTimeDelta);
	void Enter_SwordAni(CAnimator* pAnimator, _float fTimeDelta);
	void Check_EndAttackState(CKirby* pKirby, CAnimator* pAnimator, _float fTimeDelta);

	void Update_ChargeTime(_float fTimeDelta);

	void MoveLock_Ratio(_float fRatio, _float fRatioStart, _float fRatioEnd);
	void SetSpeed_Ratio(_float fRatio, _float fRatioStart, _float fRatioEnd, CMovement_Child* pMovement, _float fSpeed);
	void Charge_Start(CKirby* pKirby, CMovement_Child* pMovement);

public:
	static CKirby_Ability_Sword* Create();
private:
	virtual void Free() override;
};

NS_END