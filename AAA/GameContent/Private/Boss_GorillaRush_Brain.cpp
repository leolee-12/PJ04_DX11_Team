#include "Boss_GorillaRush_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Monster_Movement.h"
#include "MultiHitBoxPart.h"
#include "Boss_Gorilla_Body.h"

CBTNode* CBoss_GorillaRush_Brain::Build_PhaseTree(_int)
{
    return CBTReactiveSelector::Create({
        CBTSequence::Create({ Make_CooldownGate(), Make_InRange(),
            CBTReactiveSelector::Create({
                CBTSequence::Create({ Make_Facing(), Make_AttackDecision(), Make_ResetTimer() }),
                Make_Turn(),
            }),
        }),
        Make_Chase(),
        });
}

CBTNode* CBoss_GorillaRush_Brain::Make_ArmSwing(_bool bR)
{
    const string s = bR ? "R" : "L";
    const _int idx = bR ? CBoss_Gorilla_Body::GHB_RHAND : CBoss_Gorilla_Body::GHB_LHAND;
    return CBTSequence::Create({
        Clip("ArmSwingChargeStart" + s, SPD), Loop("ArmSwingChargeLoop" + s, CHARGE_TIME, SPD),
        SetHitBox(idx, true), Clip("ArmSwing" + s, SPD), SetHitBox(idx, false),
        CBTAction::Create([this](CBlackboard*, _float) { ++m_iMelee; return BT_STATUS::SUCCESS; }),
        });
}

CBTNode* CBoss_GorillaRush_Brain::Make_ArmSpin()
{
    auto t = make_shared<float>(0.f); auto on = make_shared<bool>(false);
    auto* pSpin = CBTAction::Create(
        [this, t, on](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*on) { m_pOwner->Get_BodyAnimator()->Play("ArmSpin", true, true, 0.2f, SPD); *on = true; }
            *t += dt;
            _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0));
            if (!XMVector3Equal(XMLoadFloat3(&d), XMVectorZero())) m_pOwner->Add_MoveDir(d);
            if (*t >= 6.f) { *t = 0.f; *on = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [t, on]() { *t = 0.f; *on = false; });
    return CBTSequence::Create({
        Clip("ArmSpinChargeStart", SPD), Loop("ArmSpinChargeLoop", CHARGE_TIME, SPD),
        Clip("ArmSpinStart", SPD), SetHitBox(CBoss_Gorilla_Body::GHB_SPIN, true),
        pSpin, SetHitBox(CBoss_Gorilla_Body::GHB_SPIN, false), Clip("ArmSpinEnd", SPD),
        CBTAction::Create([this](CBlackboard*, _float) { m_iMelee = 0; return BT_STATUS::SUCCESS; }),
        });
}

CBTNode* CBoss_GorillaRush_Brain::Make_Stamp(_bool bR)
{
    const string s = bR ? "R" : "L";
    return CBTSequence::Create({ Clip("StampStart" + s, SPD), Loop("StampWait" + s, CHARGE_TIME, SPD), Clip("Stamp" + s, SPD) });
}

CBTNode* CBoss_GorillaRush_Brain::Make_StampPattern()
{
    return CBTSequence::Create({ Make_Stamp(true), Make_Stamp(false), Clip("StampFinish", SPD),
        CBTAction::Create([this](CBlackboard*, _float) { ++m_iMelee; return BT_STATUS::SUCCESS; }) });
}

CBTNode* CBoss_GorillaRush_Brain::Make_Turn()
{
    return CBTAction::Create([this](CBlackboard* pBB, _float dt) -> BT_STATUS {
        m_pOwner->Get_BodyAnimator()->Play("Wait", true, false, 0.2f, SPD);   // 원본대로 매 프레임 재생(restart=false라 no-op)
        _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0)); _vector vT = XMLoadFloat3(&d);
        if (XMVector3Equal(vT, XMVectorZero())) return BT_STATUS::SUCCESS;
        if (IsFacing(vT, FACE_DOT)) return BT_STATUS::SUCCESS;
        RotateYawTo(vT, TURN_DEG, dt);
        return BT_STATUS::RUNNING; });
}

CBTNode* CBoss_GorillaRush_Brain::Make_Chase()
{
    return CBTAction::Create([this](CBlackboard* pBB, _float dt) -> BT_STATUS {
        m_pOwner->Get_BodyAnimator()->Play("Walk", true, false, 0.2f, SPD);
        _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0)); _vector vT = XMLoadFloat3(&d);
        CTransform* tf = m_pOwner->Get_Transform();
        if (!XMVector3Equal(vT, XMVectorZero())) {
            _vector vL = XMVector3Normalize(XMVectorSetY(tf->Get_State(STATE::LOOK), 0.f));
            _float dot = XMVectorGetX(XMVector3Dot(vL, vT));
            _float cr = XMVectorGetZ(vL) * XMVectorGetX(vT) - XMVectorGetX(vL) * XMVectorGetZ(vT);
            _float yaw = atan2f(cr, dot), st = XMConvertToRadians(TURN_DEG) * dt, ap = (fabsf(yaw) <= st) ? yaw : (yaw > 0 ? st : -st);
            tf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), ap));
            if (dot > 0.3f) { _float3 f; XMStoreFloat3(&f, XMVector3Normalize(XMVectorSetY(tf->Get_State(STATE::LOOK), 0.f))); m_pOwner->Add_MoveDir(f); }
        }
        return BT_STATUS::RUNNING; });
}

CBTNode* CBoss_GorillaRush_Brain::Make_Rand2(CBTNode* a, CBTNode* b)
{
    return CBTSelector::Create({
        CBTSequence::Create({ CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }), a }), b });
}

CBTNode* CBoss_GorillaRush_Brain::Make_AttackDecision()
{
    return CBTSelector::Create({
        CBTSequence::Create({ CBTCondition::Create([this](CBlackboard*) { return m_iMelee >= 3; }), Make_ArmSpin() }),
        CBTSequence::Create({ CBTCondition::Create([](CBlackboard* b) { return b->Get<_float>("DistToTarget", FLT_MAX) <= CLOSE_RANGE; }), Make_StampPattern() }),
        CBTSequence::Create({ CBTCondition::Create([](CBlackboard* b) { return b->Get<_float>("DistToTarget", FLT_MAX) <= SWING_RANGE; }), Make_Rand2(Make_ArmSwing(false), Make_ArmSwing(true)) }),
        });
}

CBTNode* CBoss_GorillaRush_Brain::Make_CooldownGate()
{
    return CBTAction::Create([this](CBlackboard*, _float dt) { m_fAtkTmr += dt; return m_fAtkTmr >= ATK_INTERVAL ? BT_STATUS::SUCCESS : BT_STATUS::FAILURE; });
}

CBTNode* CBoss_GorillaRush_Brain::Make_ResetTimer()
{
    return CBTAction::Create([this](CBlackboard*, _float) { m_fAtkTmr = 0.f; return BT_STATUS::SUCCESS; });
}

CBTNode* CBoss_GorillaRush_Brain::Make_Facing()
{
    return CBTCondition::Create([this](CBlackboard* pBB) {
        _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0));
        _vector vL = XMVector3Normalize(XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
        return XMVectorGetX(XMVector3Dot(vL, XMLoadFloat3(&d))) >= FACE_DOT; });
}

CBTNode* CBoss_GorillaRush_Brain::Make_InRange()
{
    return CBTCondition::Create([this](CBlackboard* b) { return m_iMelee >= 3 || b->Get<_float>("DistToTarget", FLT_MAX) <= SWING_RANGE; });
}

CBoss_GorillaRush_Brain* CBoss_GorillaRush_Brain::Create(CMonster* pOwner)
{
    auto* p = new CBoss_GorillaRush_Brain();
    if (FAILED(p->Initialize_Trees(pOwner))) { MSG_BOX("Failed: CBoss_GorillaRush_Brain"); Safe_Release(p); }
    return p;
}
void CBoss_GorillaRush_Brain::Free() { __super::Free(); }