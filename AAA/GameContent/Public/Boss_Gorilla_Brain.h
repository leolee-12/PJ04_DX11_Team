#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)

class CBoss_Gorilla_Brain final : public CBoss_Brain
{
private:
    CBoss_Gorilla_Brain() = default;
    virtual ~CBoss_Gorilla_Brain() = default;

protected:
    virtual _int     Get_PhaseCount() const override { return 2; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    _int   m_iMeleeSinceThrow = { 0 };
    _float m_fAtkTimer = { 0.f };

    static constexpr _float CLOSE_RANGE = 6.f, SWING_RANGE = 25.f, FACE_DOT = 0.86f,
        TURN_DEG = 90.f, CHARGE_TIME = 0.2f, SPD = 1.5f, ATK_INTERVAL = 2.f, STOP_RANGE = 5.f;
    static constexpr _uint CATCH_PCT = 20;

private:
    CBTNode* Make_ArmSwing(_bool bRight);
    CBTNode* Make_ArmSpin();
    CBTNode* Make_Stamp(_bool bRight);
    CBTNode* Make_StampSided();
    CBTNode* Make_StampPattern();
    CBTNode* Make_ThrowRock();
    CBTNode* Make_Catch();
    CBTNode* Make_JumpToTarget();
    CBTNode* Make_MeleeCount();
    CBTNode* Make_Rand2(CBTNode* pA, CBTNode* pB);
    CBTNode* Make_AttackDecision(_int iPhase);
    CBTNode* Make_Chase();
    CBTNode* Make_HoldGround();
    CBTNode* Make_TurnToTarget();
    CBTNode* Make_Facing();
    CBTNode* Make_StandStare();
    CBTNode* Make_CooldownGate();
    CBTNode* Make_ResetTimer();
    CBTNode* Make_Attackable(_int iPhase);

public:
    static CBoss_Gorilla_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END