#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Ability_ToyHammer final : public CKirby_Ability
{
private:
    enum class TOY_HAMMER_STATE
    {
        ATTACK_START, ATTACK, ATTACK_END, ATTACK_MISS, ATTACK_FINAL,
        CHARGE_START, CHARGING, CHARGE_ATTACK_1, CHARGE_ATTACK_2, CHARGE_ATTACK_3, CHARGE_ATTACK_4,
        WHEELHAMMER, WHEELHAMMER_END, WHEELHAMMER_FALL, REBOUND,
        TOY_HAMER_STATE_END
    };

    enum class CHARGE_LEVEL { LV1, LV2, LV3, LV4 };

    enum class CHARGE_ANI_STATE { NONE, WAIT, MOVE, JUMP_START, AIR, JUMP_END };

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

    virtual _bool Handle_BodyAnimEvent(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase) override;

private:
    TOY_HAMMER_STATE m_eToyHammerState{ TOY_HAMMER_STATE::TOY_HAMER_STATE_END };
    TOY_HAMMER_STATE m_eToyHammerStartState{ TOY_HAMMER_STATE::TOY_HAMER_STATE_END };

    _bool m_bReserveNextAttack{};
    _bool m_bIsCharging{};
    _bool m_bIsHit{};

    _bool m_bAttackEndOverlayApplied{};
    _bool m_bAttackFinalEndOverlayApplied{};
    _bool m_bAttackFinalAddVelocity{};
    _bool m_bWheelHammerEndOverlayApplied{};

    _uint m_iNormalAttackCount{};

    _float m_fChargeTime{};
    CHARGE_LEVEL m_eChargeLevel{};

    _bool m_bWheelHammerPressing{};

    CHARGE_ANI_STATE m_eChargeAniState{};

    CEffect_Container* m_pHammerFire{};
    CEffect_Container* m_pHammerFireSwing{};

private:
    void Change_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eNext);
    void Enter_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eState);
    void Update_ToyHammerState(CKirby* pKirby, _float fTimeDelta);
    void Exit_ToyHammerState(CKirby* pKirby, TOY_HAMMER_STATE eState);

    void Change_ChargeAniState(CKirby* pKirby, CHARGE_ANI_STATE eState);
    void Update_ChargeOverlayAni(CKirby* pKirby, _bool bUseMoveAni, _float fTimeDelta);

    void MoveLookDir(CKirby* pKirby, _float fSpeed);

    _bool Check_HammerHitGround(CKirby* pKirby, _float fNormalY = 0.f, _float fExtraDistance = 0.05f, _float3* pOutHitPos = nullptr);

public:
    static CKirby_Ability_ToyHammer* Create();
private:
    virtual void Free() override;
};

NS_END
