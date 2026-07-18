#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)
class CBoss_GorillaRush_Brain final : public CBoss_Brain
{
private:
    CBoss_GorillaRush_Brain() = default;
    virtual ~CBoss_GorillaRush_Brain() = default;
protected:
    virtual _int     Get_PhaseCount() const override { return 1; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    static constexpr _float CLOSE_RANGE = 6.f, SWING_RANGE = 25.f, FACE_DOT = 0.86f,
        TURN_DEG = 90.f, CHARGE_TIME = 0.2f, SPD = 1.5f, ATK_INTERVAL = 2.f;

    _int   m_iMelee = { 0 };
    _float m_fAtkTmr = { 0.f };

private:
    CBTNode* Make_ArmSwing(_bool bR);
    CBTNode* Make_ArmSpin();
    CBTNode* Make_Stamp(_bool bR);
    CBTNode* Make_StampPattern();
    CBTNode* Make_Turn();
    CBTNode* Make_Chase();
    CBTNode* Make_Rand2(CBTNode* a, CBTNode* b);
    CBTNode* Make_AttackDecision();
    CBTNode* Make_CooldownGate();
    CBTNode* Make_ResetTimer();
    CBTNode* Make_Facing();
    CBTNode* Make_InRange();
public:
    static CBoss_GorillaRush_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};
NS_END