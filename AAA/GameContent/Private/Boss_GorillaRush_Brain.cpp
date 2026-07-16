#include "Boss_GorillaRush_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Animator.h"
#include "Transform.h"
#include "Monster_Movement.h"
#include "MultiHitBoxPart.h"
#include "Boss_Gorilla_Body.h"

CBTNode* CBoss_GorillaRush_Brain::Build_PhaseTree(_int)
{
    const _float fCloseRange = 6.f, fSwingRange = 25.f, fFacingDot = 0.86f;
    const _float fTurnDeg = 90.f, fChargeTime = 0.2f, fSpd = 1.5f, fAtkInterval = 2.f;

    auto iMelee = make_shared<int>(0);
    auto fAtkTmr = make_shared<float>(0.f);

    auto Anim = [this]() -> CAnimator* { return m_pOwner->Get_BodyAnimator(); };
    auto OneShot = [&](const string& c) { return CBTPlayClip::Create(Anim, { c, false, 0.f, fSpd }); };
    auto HoldLoop = [&](const string& c, _float h) { return CBTPlayClip::Create(Anim, { c, true, h, fSpd }); };
    auto HitBox = [this](_int i, _bool on) { return CBTAction::Create([this, i, on](CBlackboard*, _float) {
        if (auto* p = static_cast<CBoss*>(m_pOwner)->Get_HitBoxPart()) p->Enable_HitBox(i, on);
        return BT_STATUS::SUCCESS; }); };

    auto MakeArmSwing = [&](_bool bR)->CBTNode* {
        const string s = bR ? "R" : "L";
        const _int idx = bR ? CBoss_Gorilla_Body::GHB_RHAND : CBoss_Gorilla_Body::GHB_LHAND;
        return CBTSequence::Create({
            OneShot("ArmSwingChargeStart" + s), HoldLoop("ArmSwingChargeLoop" + s, fChargeTime),
            HitBox(idx,true), OneShot("ArmSwing" + s), HitBox(idx,false),
            [&,iMelee]() { return CBTAction::Create([iMelee](CBlackboard*,_float) { ++(*iMelee); return BT_STATUS::SUCCESS; }); }(),
            });
        };

    auto MakeArmSpin = [&]()->CBTNode* {
        auto t = make_shared<float>(0.f); auto on = make_shared<bool>(false);
        auto* pSpin = CBTAction::Create(
            [this, t, on, fSpd](CBlackboard* pBB, _float dt)->BT_STATUS {
                if (!*on) { m_pOwner->Get_BodyAnimator()->Play("ArmSpin", true, true, 0.2f, fSpd); *on = true; }
                *t += dt;
                _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0));
                if (!XMVector3Equal(XMLoadFloat3(&d), XMVectorZero())) m_pOwner->Add_MoveDir(d);
                if (*t >= 6.f) { *t = 0.f; *on = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            }, [t, on]() { *t = 0.f; *on = false; });
        return CBTSequence::Create({
            OneShot("ArmSpinChargeStart"), HoldLoop("ArmSpinChargeLoop", fChargeTime),
            OneShot("ArmSpinStart"), HitBox(CBoss_Gorilla_Body::GHB_SPIN,true),
            pSpin, HitBox(CBoss_Gorilla_Body::GHB_SPIN,false), OneShot("ArmSpinEnd"),
            CBTAction::Create([iMelee](CBlackboard*,_float) { *iMelee = 0; return BT_STATUS::SUCCESS; }),
            });
        };
    auto MakeStamp = [&](_bool bR)->CBTNode* { const string s = bR ? "R" : "L";
    return CBTSequence::Create({ OneShot("StampStart" + s), HoldLoop("StampWait" + s,fChargeTime), OneShot("Stamp" + s) }); };
    auto MakeStampPattern = [&]()->CBTNode* {
        return CBTSequence::Create({ MakeStamp(true), MakeStamp(false), OneShot("StampFinish"),
            CBTAction::Create([iMelee](CBlackboard*,_float) { ++(*iMelee); return BT_STATUS::SUCCESS; }) }); };

    auto* pTurn = CBTAction::Create([this, fTurnDeg, fFacingDot, fSpd](CBlackboard* pBB, _float dt)->BT_STATUS {
        m_pOwner->Get_BodyAnimator()->Play("Wait", true, false, 0.2f, fSpd);
        _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0)); _vector vT = XMLoadFloat3(&d);
        if (XMVector3Equal(vT, XMVectorZero())) return BT_STATUS::SUCCESS;
        CTransform* tf = m_pOwner->Get_Transform();
        _vector vL = XMVector3Normalize(XMVectorSetY(tf->Get_State(STATE::LOOK), 0.f));
        _float dot = XMVectorGetX(XMVector3Dot(vL, vT)); if (dot >= fFacingDot) return BT_STATUS::SUCCESS;
        _float cr = XMVectorGetZ(vL) * XMVectorGetX(vT) - XMVectorGetX(vL) * XMVectorGetZ(vT);
        _float yaw = atan2f(cr, dot), st = XMConvertToRadians(fTurnDeg) * dt, ap = (fabsf(yaw) <= st) ? yaw : (yaw > 0 ? st : -st);
        tf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), ap)); return BT_STATUS::RUNNING; });

    auto* pChase = CBTAction::Create([this, fTurnDeg, fSpd](CBlackboard* pBB, _float dt)->BT_STATUS {
        m_pOwner->Get_BodyAnimator()->Play("Walk", true, false, 0.2f, fSpd);
        _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0)); _vector vT = XMLoadFloat3(&d);
        CTransform* tf = m_pOwner->Get_Transform();
        if (!XMVector3Equal(vT, XMVectorZero())) {
            _vector vL = XMVector3Normalize(XMVectorSetY(tf->Get_State(STATE::LOOK), 0.f));
            _float dot = XMVectorGetX(XMVector3Dot(vL, vT));
            _float cr = XMVectorGetZ(vL) * XMVectorGetX(vT) - XMVectorGetX(vL) * XMVectorGetZ(vT);
            _float yaw = atan2f(cr, dot), st = XMConvertToRadians(fTurnDeg) * dt, ap = (fabsf(yaw) <= st) ? yaw : (yaw > 0 ? st : -st);
            tf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), ap));
            if (dot > 0.3f) { _float3 f; XMStoreFloat3(&f, XMVector3Normalize(XMVectorSetY(tf->Get_State(STATE::LOOK), 0.f))); m_pOwner->Add_MoveDir(f); }
        }
        return BT_STATUS::RUNNING; });

    auto Rand2 = [&](CBTNode* a, CBTNode* b) { return CBTSelector::Create({
        CBTSequence::Create({ CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }), a }), b }); };

    CBTNode* pAtk = CBTSelector::Create({
        CBTSequence::Create({ CBTCondition::Create([iMelee](CBlackboard*) { return *iMelee >= 3; }), MakeArmSpin() }),
        CBTSequence::Create({ CBTCondition::Create([fCloseRange](CBlackboard* b) { return b->Get<_float>("DistToTarget",FLT_MAX) <= fCloseRange; }), MakeStampPattern() }),
        CBTSequence::Create({ CBTCondition::Create([fSwingRange](CBlackboard* b) { return b->Get<_float>("DistToTarget",FLT_MAX) <= fSwingRange; }), Rand2(MakeArmSwing(false),MakeArmSwing(true)) }),
        });

    auto* pCdGate = CBTAction::Create([fAtkTmr, fAtkInterval](CBlackboard*, _float dt) { *fAtkTmr += dt; return *fAtkTmr >= fAtkInterval ? BT_STATUS::SUCCESS : BT_STATUS::FAILURE; });
    auto* pReset = CBTAction::Create([fAtkTmr](CBlackboard*, _float) { *fAtkTmr = 0.f; return BT_STATUS::SUCCESS; });
    auto* pFacing = CBTCondition::Create([this, fFacingDot](CBlackboard* pBB) {
        _float3 d = pBB->Get<_float3>("DirToTarget", _float3(0, 0, 0));
        _vector vL = XMVector3Normalize(XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
        return XMVectorGetX(XMVector3Dot(vL, XMLoadFloat3(&d))) >= fFacingDot; });
    auto* pInRange = CBTCondition::Create([iMelee, fSwingRange](CBlackboard* b) {
        return *iMelee >= 3 || b->Get<_float>("DistToTarget", FLT_MAX) <= fSwingRange; });

    return CBTReactiveSelector::Create({
        CBTSequence::Create({ pCdGate, pInRange,
            CBTReactiveSelector::Create({
                CBTSequence::Create({ pFacing, pAtk, pReset }),
                pTurn,
            }),
        }),
        pChase,
        });
}

CBoss_GorillaRush_Brain* CBoss_GorillaRush_Brain::Create(CMonster* pOwner)
{
    auto* p = new CBoss_GorillaRush_Brain();
    if (FAILED(p->Initialize_Trees(pOwner))) { MSG_BOX("Failed: CBoss_GorillaRush_Brain"); Safe_Release(p); }
    return p;
}
void CBoss_GorillaRush_Brain::Free() { __super::Free(); }