#include "Boss_Gorilla_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Monster_Movement.h"
#include "MultiHitBoxPart.h"
#include "Boss_Gorilla_Body.h"
#include "Boss_Gorilla.h"

namespace 
{
    constexpr const _tchar* SND_ARMSPIN = L"CharaBossGorilla_ArmSpin.wav";
}

CBTNode* CBoss_Gorilla_Brain::Build_PhaseTree(_int iPhase)
{
    return CBTReactiveSelector::Create({
        CBTSequence::Create({
            Make_CooldownGate(),
            CBTSequence::Create({
                Make_CooldownGate(),
                Make_Attackable(iPhase),
                CBTReactiveSelector::Create({
                    CBTReactiveSelector::Create({
                        CBTSequence::Create({ Make_Facing(), Make_AttackDecision(iPhase), Make_ResetTimer() }),
                        Make_TurnToTarget(),
                    }),
                }),
            }),
        }),
        Make_StandStare(),
        Make_Chase(),
        });
}

// ============ 공격 ============
CBTNode* CBoss_Gorilla_Brain::Make_ArmSwing(_bool bRight)
{
    const string s = bRight ? "R" : "L";
    const _int idx = bRight ? CBoss_Gorilla_Body::GHB_RHAND : CBoss_Gorilla_Body::GHB_LHAND;
    return CBTSequence::Create({
        Clip("ArmSwingChargeStart" + s, SPD),
        Loop("ArmSwingChargeLoop" + s, CHARGE_TIME, SPD),
        SetHitBox(idx, true),
        Clip("ArmSwing" + s, SPD),
        SetHitBox(idx, false),
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_ArmSpin()
{
    const _float fSpinDuration = 10.f, fSpinTurnDeg = 120.f;
    auto fSpinT = make_shared<_float>(0.f);
    auto bSpin = make_shared<bool>(false);
    auto* pSpinChase = CBTAction::Create(
        [this, fSpinT, bSpin, fSpinDuration, fSpinTurnDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bSpin) {
                m_pOwner->Get_BodyAnimator()->Play("ArmSpin", true, true, 0.2f, SPD);
                m_pOwner->Play_LoopSFX(TEXT("CharaBossGorilla_ArmSpin.wav"), 0.25f);
                *bSpin = true;
            }
            *fSpinT += dt;
            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vToTgt = XMLoadFloat3(&vDir);
            if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                m_pOwner->Add_MoveDir(vDir);
                CTransform* pTf = m_pOwner->Get_Transform();
                _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                _float fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt) - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                _float fYaw = atan2f(fCross, fDot);
                _float fStep = XMConvertToRadians(fSpinTurnDeg) * dt;
                _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
            }
            if (*fSpinT >= fSpinDuration) { m_pOwner->Stop_LoopSFX(SND_ARMSPIN); *fSpinT = 0.f; *bSpin = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [this, fSpinT, bSpin]() {
            if (!*bSpin) return;
            *fSpinT = 0.f; *bSpin = false;
            m_pOwner->Stop_LoopSFX(SND_ARMSPIN);
        });

    return CBTSequence::Create({
        Clip("ArmSpinChargeStart", SPD),
        Loop("ArmSpinChargeLoop", CHARGE_TIME, SPD),
        Clip("ArmSpinStart", SPD),
        SetHitBox(CBoss_Gorilla_Body::GHB_SPIN, true),
        pSpinChase,
        SetHitBox(CBoss_Gorilla_Body::GHB_SPIN, false),
        Clip("ArmSpinEnd", SPD),
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_Stamp(_bool bRight)
{
    const string s = bRight ? "R" : "L";
    return CBTSequence::Create({
        Clip("StampStart" + s, SPD),
        Loop("StampWait" + s, CHARGE_TIME, SPD),
        Clip("Stamp" + s, SPD),
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_StampSided()
{
    return CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard* pBB) {
                _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                _vector vT = XMLoadFloat3(&vDir);
                _vector vL = XMVector3Normalize(XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
                _float fCross = XMVectorGetZ(vL) * XMVectorGetX(vT) - XMVectorGetX(vL) * XMVectorGetZ(vT);
                return fCross >= 0.f; }),
            Make_Stamp(true),
        }),
        Make_Stamp(false),
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_StampPattern()
{
    return CBTSequence::Create({ Make_StampSided(), Make_StampSided(), Clip("StampFinish", SPD) });
}

CBTNode* CBoss_Gorilla_Brain::Make_ThrowRock()
{
    return CBTSequence::Create({
        FaceWindup("ThrowRockReady", TURN_DEG, SPD),
        FaceWindup("ThrowRock", TURN_DEG, SPD),
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_Catch()
{
    auto bAtkOn = make_shared<bool>(false);
    auto* pCatchAttack = CBTAction::Create(
        [this, bAtkOn](CBlackboard*, _float) -> BT_STATUS {
            CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
            auto* pGorilla = static_cast<CBoss_Gorilla*>(m_pOwner);
            if (!*bAtkOn) {
                *bAtkOn = true;
                pGorilla->Reset_CatchHit();
                pAnim->Play("CatchAttack", false, true, 0.1f, SPD);
                pGorilla->Get_HitBoxPart()->Enable_HitBox(CBoss_Gorilla_Body::GHB_CATCH, true);
            }
            if (pGorilla->Is_CatchHit()) {
                pGorilla->Get_HitBoxPart()->Enable_HitBox(CBoss_Gorilla_Body::GHB_CATCH, false);
                *bAtkOn = false; return BT_STATUS::SUCCESS;
            }
            if (pAnim->Is_Finished()) {
                pGorilla->Get_HitBoxPart()->Enable_HitBox(CBoss_Gorilla_Body::GHB_CATCH, false);
                *bAtkOn = false; return BT_STATUS::SUCCESS;
            }
            if (!pGorilla->Is_CatchHit()) {
                CTransform* pTf = m_pOwner->Get_Transform();
                _float3 vFwd{}; XMStoreFloat3(&vFwd, XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f)));
                m_pOwner->Add_MoveDir(vFwd);
            }
            return BT_STATUS::RUNNING;
        },
        [this, bAtkOn]() {
            *bAtkOn = false;
            static_cast<CBoss_Gorilla*>(m_pOwner)->Get_HitBoxPart()->Enable_HitBox(CBoss_Gorilla_Body::GHB_CATCH, false);
        });

    auto bWaitOn = make_shared<bool>(false);
    auto* pQTEWait = CBTAction::Create(
        [this, bWaitOn](CBlackboard*, _float fDt) -> BT_STATUS {
            auto* pGorilla = static_cast<CBoss_Gorilla*>(m_pOwner);
            if (!*bWaitOn) { *bWaitOn = true; m_pOwner->Get_BodyAnimator()->Play("CatchSuccessWait", true, true, 0.1f, 1.f); }
            if (pGorilla->Is_QTEEscaped()) { *bWaitOn = false; return BT_STATUS::SUCCESS; }
            if (pGorilla->Tick_QTETimer(fDt)) { *bWaitOn = false; return BT_STATUS::FAILURE; }
            return BT_STATUS::RUNNING;
        },
        [bWaitOn]() { *bWaitOn = false; });

    auto* pReleaseEscape = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Gorilla*>(m_pOwner)->Fire_Release(KIRBY_ATTACHMENT_END_REASON::GORILLA_COMBAT_ESCAPE);
        return BT_STATUS::SUCCESS; });

    auto bThrowOn = make_shared<bool>(false);
    auto* pThrow = CBTAction::Create(
        [this, bThrowOn](CBlackboard*, _float) -> BT_STATUS {
            CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
            if (!*bThrowOn) { *bThrowOn = true; pAnim->Play("CatchThrow", false, true, 0.1f, SPD); }
            if (pAnim->Is_Finished()) {
                *bThrowOn = false;
                static_cast<CBoss_Gorilla*>(m_pOwner)->Fire_Release(KIRBY_ATTACHMENT_END_REASON::GORILLA_COMBAT_THROWN);
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bThrowOn]() { *bThrowOn = false; });

    return CBTSequence::Create({
        Clip("CatchReadyStart", SPD),
        Clip("CatchReady", SPD),
        pCatchAttack,
        CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([this](CBlackboard*) { return static_cast<CBoss_Gorilla*>(m_pOwner)->Is_CatchHit(); }),
                Clip("CatchSuccessB", SPD),
                CBTSelector::Create({
                    CBTSequence::Create({ pQTEWait, pReleaseEscape, Clip("CatchRelease", SPD) }),
                    pThrow,
                }),
            }),
            Clip("CatchFailure", SPD),
        }),
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_JumpToTarget()
{
    auto bAir = make_shared<bool>(false);
    const _float fJumpTime = 1.5f, fJumpHeight = 10.f;
    auto* pAir = CBTAction::Create(
        [this, bAir, fJumpTime, fJumpHeight](CBlackboard*, _float) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            if (!*bAir) {
                m_pOwner->Get_BodyAnimator()->Play("Jump", false, true, 0.1f, SPD);
                _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
                _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                _vector vToTgt = XMVectorSetY(vTgt - vSelf, 0.f);
                if (XMVectorGetX(XMVector3LengthSq(vToTgt)) > 1e-6f) vToTgt = XMVector3Normalize(vToTgt);
                vTgt = vTgt - vToTgt * 5.f;
                mv->Begin_JumpArc(vTgt, fJumpTime, fJumpHeight);
                *bAir = true;
            }
            if (!mv->Is_JumpArc()) { *bAir = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [bAir]() { *bAir = false; });

    return CBTSequence::Create({ Clip("JumpStart", SPD), pAir, Clip("Landing", SPD), Clip("LandingEnd", SPD) });
}

CBTNode* CBoss_Gorilla_Brain::Make_MeleeCount()
{
    return CBTAction::Create([this](CBlackboard*, _float) { ++m_iMeleeSinceThrow; return BT_STATUS::SUCCESS; });
}

CBTNode* CBoss_Gorilla_Brain::Make_Rand2(CBTNode* pA, CBTNode* pB)
{
    return CBTSelector::Create({
        CBTSequence::Create({ CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }), pA }),
        pB,
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_AttackDecision(_int iPhase)
{
    return CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard*) { return m_iMeleeSinceThrow >= 3; }),
            (iPhase == 0 ? Make_JumpToTarget() : Make_ArmSpin()),
            CBTAction::Create([this](CBlackboard*, _float) { m_iMeleeSinceThrow = 0; return BT_STATUS::SUCCESS; }),
        }),
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard* pBB) {
                return pBB->Get<_float>("DistToTarget", FLT_MAX) <= SWING_RANGE && (rand() % 100) < (int)CATCH_PCT; }),
            Make_Catch(), Make_MeleeCount(),
        }),
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard* pBB) { return pBB->Get<_float>("DistToTarget", FLT_MAX) <= CLOSE_RANGE; }),
            Make_StampPattern(), Make_MeleeCount(),
        }),
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard* pBB) { return pBB->Get<_float>("DistToTarget", FLT_MAX) <= SWING_RANGE; }),
            Make_Rand2(Make_ArmSwing(false), Make_ArmSwing(true)), Make_MeleeCount(),
        }),
        CBTSequence::Create({
            CBTCondition::Create([iPhase](CBlackboard* pBB) { return iPhase == 0 && pBB->Get<_float>("DistToTarget", FLT_MAX) > SWING_RANGE; }),
            Make_ThrowRock(), Make_MeleeCount(),
        }),
        });
}

// ============ 이동/상태 ============
CBTNode* CBoss_Gorilla_Brain::Make_Chase()
{
    auto bMove = make_shared<bool>(false);
    return CBTAction::Create(
        [this, bMove](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bMove) { m_pOwner->Get_BodyAnimator()->Play("Walk", true, true, 0.2f, SPD); *bMove = true; }
            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vToTgt = XMLoadFloat3(&vDir);
            CTransform* pTf = m_pOwner->Get_Transform();
            _float fDot = 1.f;
            if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt) - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                _float fYaw = atan2f(fCross, fDot);
                _float fStep = XMConvertToRadians(TURN_DEG) * dt;
                _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
            }
            if (fDot > 0.3f) {
                _vector vFwd = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                _float3 vMove; XMStoreFloat3(&vMove, vFwd);
                m_pOwner->Add_MoveDir(vMove);
            }
            return BT_STATUS::RUNNING;
        },
        [bMove]() { *bMove = false; });
}

CBTNode* CBoss_Gorilla_Brain::Make_HoldGround()
{
    auto bHold = make_shared<bool>(false);
    return CBTAction::Create(
        [this, bHold](CBlackboard*, _float) -> BT_STATUS {
            if (!*bHold) { m_pOwner->Get_BodyAnimator()->Play("Wait", true, true); *bHold = true; }
            return BT_STATUS::RUNNING;
        },
        [bHold]() { *bHold = false; });
}

CBTNode* CBoss_Gorilla_Brain::Make_TurnToTarget()
{
    auto bTurn = make_shared<bool>(false);
    return CBTAction::Create(
        [this, bTurn](CBlackboard* pBB, _float fDt) -> BT_STATUS {
            if (!*bTurn) { m_pOwner->Get_BodyAnimator()->Play("Wait", true, true); *bTurn = true; }
            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vToTgt = XMLoadFloat3(&vDir);
            if (XMVector3Equal(vToTgt, XMVectorZero())) { *bTurn = false; return BT_STATUS::SUCCESS; }
            if (IsFacing(vToTgt, FACE_DOT)) { *bTurn = false; return BT_STATUS::SUCCESS; }
            RotateYawTo(vToTgt, TURN_DEG, fDt);
            return BT_STATUS::RUNNING;
        },
        [bTurn]() { *bTurn = false; });
}

CBTNode* CBoss_Gorilla_Brain::Make_Facing()
{
    return CBTCondition::Create([this](CBlackboard* pBB) {
        _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
        _vector vToTgt = XMLoadFloat3(&vDir);
        _vector vLook = XMVector3Normalize(XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
        return XMVectorGetX(XMVector3Dot(vLook, vToTgt)) >= FACE_DOT; });
}

CBTNode* CBoss_Gorilla_Brain::Make_StandStare()
{
    auto bHeld = make_shared<bool>(false);
    return CBTSequence::Create({
        CBTCondition::Create([this, bHeld](CBlackboard* pBB) {
            _float d = pBB->Get<_float>("DistToTarget", FLT_MAX);
            _float fEnter = STOP_RANGE, fExit = STOP_RANGE + 2.f;
            _bool bStay = *bHeld ? (d <= fExit) : (d <= fEnter);
            *bHeld = bStay;
            return bStay; }),
        Make_HoldGround(),
        });
}

CBTNode* CBoss_Gorilla_Brain::Make_CooldownGate()
{
    return CBTAction::Create([this](CBlackboard*, _float dt) -> BT_STATUS {
        m_fAtkTimer += dt;
        return (m_fAtkTimer >= ATK_INTERVAL) ? BT_STATUS::SUCCESS : BT_STATUS::FAILURE; });
}

CBTNode* CBoss_Gorilla_Brain::Make_ResetTimer()
{
    return CBTAction::Create([this](CBlackboard*, _float) { m_fAtkTimer = 0.f; return BT_STATUS::SUCCESS; });
}

CBTNode* CBoss_Gorilla_Brain::Make_Attackable(_int iPhase)
{
    return CBTCondition::Create([this, iPhase](CBlackboard* pBB) {
        if (m_iMeleeSinceThrow >= 3) return true;
        _float d = pBB->Get<_float>("DistToTarget", FLT_MAX);
        if (d <= SWING_RANGE) return true;
        if (iPhase == 0 && d > SWING_RANGE) return true;
        return false; });
}

CBoss_Gorilla_Brain* CBoss_Gorilla_Brain::Create(CMonster* pOwner)
{
    CBoss_Gorilla_Brain* pInstance = new CBoss_Gorilla_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner)))
    {
        MSG_BOX("Failed to Created : CBoss_Gorilla_Brain"); Safe_Release(pInstance);
    }
    return pInstance;
}
void CBoss_Gorilla_Brain::Free() { __super::Free(); }