#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_ToyHammer final : public CKirby_Ability
{
private:
    enum TOY_HAMMER_STATE
    {
        ATTACK_START, ATTACK, ATTACK_END, ATTACK_FINAL,
        CHARGE_START, CHARGING, CHARGE_ATTACK_1, CHARGE_ATTACK_2, CHARGE_ATTACK_3,
        WHEELHAMMER, WHEELHAMMER_END, WHEELHAMMER_FALL,
        TOY_HAMER_STATE_END
    };

    enum CHARGE_LEVEL { LV1, LV2, LV3 };

private:
    CKirby_Ability_ToyHammer();
    virtual ~CKirby_Ability_ToyHammer() = default;

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

private:
    TOY_HAMMER_STATE m_eToyHammerState{ TOY_HAMMER_STATE::TOY_HAMER_STATE_END };

    TOY_HAMMER_STATE m_eToyHammerStartState{ TOY_HAMMER_STATE::TOY_HAMER_STATE_END };

    _bool m_bReserveNextAttack{};
    _bool m_bIsCharging{};

    _bool m_bAttackEndOverlayApplied{};
    _bool m_bWheelHammerEndOverlayApplied{};

    _uint m_iNormalAttackCount{};

    _uint m_iChargeCount{};
    CHARGE_LEVEL m_eChargeLevel{};

    _bool m_bWheelHammerPressing{};

private:
    void Change_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eNext);
    void Enter_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eState);
    void Update_ToyHammerState(CKirby* pKirby, _float fTimeDelta);
    void Exit_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eState);

public:
    static CKirby_Ability_ToyHammer* Create();
private:
    virtual void Free() override;
};

NS_END
