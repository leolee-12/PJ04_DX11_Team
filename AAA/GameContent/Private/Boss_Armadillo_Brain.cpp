#include "Boss_Armadillo_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Animator.h"
#include "Monster_Movement.h"
#include "MultiHitBoxPart.h"
#include "Boss_Armadillo.h"
#include "Boss_Armadillo_Body.h"

CBTNode* CBoss_Armadillo_Brain::Build_PhaseTree(_int)
{
    auto* pOpening = CBTSequence::Create({
        FaceWindup("AppearPartnerShort2", 360.f, SPD),
        Make_TwinDance(),
        Make_PartnerThrow(), Make_PartnerThrow(), Make_PartnerThrow(),
        CBTAction::Create([this](CBlackboard*, _float) { m_bOpeningDone = true; return BT_STATUS::SUCCESS; }),
        Make_Roll(),
        });

    auto* pLoop = CBTSequence::Create({
        Make_Rest(),
        CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([](CBlackboard* pBB) { return pBB->Get<_float>("DistToTarget", FLT_MAX) <= 25.f; }),
                Make_Catch(),
            }),
            CBTSequence::Create({ Make_PartnerThrow(), Make_PartnerThrow(), Make_PartnerThrow() }),
        }),
        Make_Rest(),
        Make_Roll(),
        });

    return CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard*) { return !m_bOpeningDone; }),
            pOpening,
        }),
        pLoop,
        });
}

CBTNode* CBoss_Armadillo_Brain::Make_Rest() { return Loop("Wait", REST_TIME, SPD); }

CBTNode* CBoss_Armadillo_Brain::Make_PartnerAnim(const _char* szClip, _bool bLoop)
{
    return CBTAction::Create([this, szClip, bLoop](CBlackboard*, _float) {
        static_cast<CBoss_Armadillo*>(m_pOwner)->Play_PartnerAnim(szClip, bLoop);
        return BT_STATUS::SUCCESS; });
}

CBTNode* CBoss_Armadillo_Brain::Make_PartnerSpinHit(_bool b)
{
    return CBTAction::Create([this, b](CBlackboard*, _float) {
        static_cast<CBoss_Armadillo*>(m_pOwner)->Enable_PartnerSpinHit(b);
        return BT_STATUS::SUCCESS; });
}

CBTNode* CBoss_Armadillo_Brain::Make_TwinDance()
{
    auto bOn = make_shared<bool>(false);
    auto fT = make_shared<_float>(0.f);
    auto fBounceCd = make_shared<_float>(0.f);
    auto iBounce = make_shared<_int>(0);
    auto vDir = make_shared<_float3>(_float3(0.f, 0.f, 1.f));
    const _float fDanceTime = 6.f; const _int iMaxBounce = 4; const _float fSpinDeg = 540.f;

    auto* pRush = CBTAction::Create(
        [this, bOn, fT, fBounceCd, iBounce, vDir, fDanceTime, iMaxBounce, fSpinDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
            auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
            if (!*bOn) {
                m_pOwner->Get_BodyAnimator()->Play("TwinDance", true, true, 0.1f, SPD);
                m_pOwner->Get_Movement()->Set_MoveSpeed(ROLL_SPEED);
                m_pOwner->Get_Movement()->Set_LockFacing(true);
                pArma->Set_TwinDanceOffset(true);
                *vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                *fT = 0.f; *iBounce = 0; *fBounceCd = 0.f; *bOn = true;
            }
            *fT += dt; *fBounceCd -= dt;
            m_pOwner->Add_MoveDir(*vDir);
            m_pOwner->Get_Transform()->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fSpinDeg) * dt));
            _float3 vN{};
            if (*fBounceCd <= 0.f && pArma->Sweep_Wall(*vDir, WALL_PROBE, &vN)) {
                _vector d = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(vDir.get()), 0.f));
                _vector n = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vN), 0.f));
                _vector r = d - n * (2.f * XMVectorGetX(XMVector3Dot(d, n)));
                XMStoreFloat3(vDir.get(), XMVector3Normalize(XMVectorSetY(r, 0.f)));
                ++(*iBounce); *fBounceCd = 0.3f;
            }
            if (*fT >= fDanceTime || *iBounce >= iMaxBounce) {
                m_pOwner->Get_Movement()->Set_MoveSpeed(WALK_SPEED);
                m_pOwner->Get_Movement()->Set_LockFacing(false);
                pArma->Set_TwinDanceOffset(false);
                *bOn = false; return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn]() {
            *bOn = false;
            m_pOwner->Get_Movement()->Set_MoveSpeed(WALK_SPEED);
            m_pOwner->Get_Movement()->Set_LockFacing(false);
            static_cast<CBoss_Armadillo*>(m_pOwner)->Set_TwinDanceOffset(false);
        });

    return CBTSequence::Create({
        Make_PartnerAnim("TwinDanceStart", false),
        FaceWindup("TwinDanceStart", 360.f, SPD),
        Make_PartnerAnim("TwinDance", true),
        SetHitBox(CBoss_Armadillo_Body::AHB_ROLL, true),
        Make_PartnerSpinHit(true),
        pRush,
        Make_PartnerSpinHit(false),
        SetHitBox(CBoss_Armadillo_Body::AHB_ROLL, false),
        Make_PartnerAnim("TwinDanceEnd", false),
        Clip("TwinDanceEnd", SPD),
        });
}

CBTNode* CBoss_Armadillo_Brain::Make_Roll()
{
    auto bOn = make_shared<bool>(false);
    auto bGroggy = make_shared<bool>(false);
    auto iState = make_shared<_int>(0);
    auto iWallHits = make_shared<_int>(0);
    auto fSegT = make_shared<_float>(0.f);
    auto fAimT = make_shared<_float>(0.f);
    auto vDir = make_shared<_float3>(_float3(0.f, 0.f, 1.f));
    const _int iGroggyHits = 3;
    const _float fSegTimeMax = 4.f, fReAimTime = 0.5f, fSnapTurnDeg = 1080.f, fBounceBackSpeed = 8.f, fBounceUpSpeed = 10.f;

    auto* pRush = CBTAction::Create(
        [this, bOn, bGroggy, iState, iWallHits, fSegT, fAimT, vDir, iGroggyHits, fSegTimeMax, fReAimTime, fSnapTurnDeg, fBounceBackSpeed, fBounceUpSpeed](CBlackboard* pBB, _float dt) -> BT_STATUS {
            auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
            if (!*bOn) {
                m_pOwner->Get_BodyAnimator()->Play("RollAttack", true, true, 0.1f, SPD);
                m_pOwner->Get_Movement()->Set_MoveSpeed(SOLO_ROLL_SPEED);
                *vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                *iState = 0; *iWallHits = 0; *fSegT = 0.f; *bGroggy = false; *bOn = true;
                pArma->Set_RutTrail(true);
            }
            if (*iState == 0) {
                *fSegT += dt; m_pOwner->Add_MoveDir(*vDir);
                _float3 vN{};
                if (pArma->Sweep_Wall(*vDir, WALL_PROBE, &vN)) {
                    ++(*iWallHits);
                    if (*iWallHits >= iGroggyHits) {
                        _vector vBounce = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vN), 0.f));
                        m_pOwner->Get_Movement()->Launch(vBounce, fBounceBackSpeed, fBounceUpSpeed);
                        m_pOwner->Get_Movement()->Set_MoveSpeed(WALK_SPEED);
                        pArma->Set_RutTrail(false);
                        *bGroggy = true; *bOn = false; return BT_STATUS::SUCCESS;
                    }
                    *iState = 1; *fAimT = 0.f;
                }
                else if (*fSegT >= fSegTimeMax) {
                    m_pOwner->Get_Movement()->Set_MoveSpeed(WALK_SPEED);
                    pArma->Set_RutTrail(false);
                    *bOn = false; return BT_STATUS::SUCCESS;
                }
            }
            else {
                *fAimT += dt;
                _float3 vTo = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                _vector vToTgt = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vTo), 0.f));
                CTransform* pTf = m_pOwner->Get_Transform();
                _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                _float fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt) - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                _float fYaw = atan2f(fCross, fDot);
                _float fStep = XMConvertToRadians(fSnapTurnDeg) * dt;
                _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                if (*fAimT >= fReAimTime) { XMStoreFloat3(vDir.get(), vToTgt); *iState = 0; *fSegT = 0.f; }
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn]() { 
            *bOn = false; 
            m_pOwner->Get_Movement()->Set_MoveSpeed(WALK_SPEED); 
            static_cast<CBoss_Armadillo*>(m_pOwner)->Set_RutTrail(false);
        });

    auto bFallOn = make_shared<bool>(false);
    auto fFallT = make_shared<_float>(0.f);
    auto* pFallLoop = CBTAction::Create(
        [this, bFallOn, fFallT](CBlackboard*, _float dt) -> BT_STATUS {
            if (!*bFallOn) { m_pOwner->Get_BodyAnimator()->Play("HitWallFall", true, true, 0.1f, SPD); *fFallT = 0.f; *bFallOn = true; }
            *fFallT += dt;
            if (*fFallT >= 0.05f && !m_pOwner->Get_Movement()->Is_Launched()) { *bFallOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [bFallOn]() { *bFallOn = false; });

    auto* pGroggy = CBTSequence::Create({
        CBTCondition::Create([bGroggy](CBlackboard*) { return *bGroggy; }),
        pFallLoop, Clip("HitWallLanding", SPD), Loop("HitWallLoop", GROGGY_TIME, SPD), Clip("HitWallEnd", SPD),
        });

    return CBTSequence::Create({
        FaceWindup("RollChargeStart", 120.f, SPD),
        Clip("RollAttackStart", SPD),
        SetHitBox(CBoss_Armadillo_Body::AHB_ROLL, true),
        pRush,
        SetHitBox(CBoss_Armadillo_Body::AHB_ROLL, false),
        CBTSelector::Create({ pGroggy, Clip("RollBrake", SPD) }),
        });
}

CBTNode* CBoss_Armadillo_Brain::Make_PartnerThrow()
{
    auto bOn = make_shared<bool>(false);
    auto* pThrow = CBTAction::Create(
        [this, bOn](CBlackboard* pBB, _float dt) -> BT_STATUS {
            auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
            CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
            if (!*bOn) { pAnim->Play("TwinRollingStart", false, true, 0.1f, SPD); pArma->Play_PartnerAnim("TwinRollingStart", false); *bOn = true; }
            if (pArma->Has_Partner()) {
                _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                _vector vToTgt = XMLoadFloat3(&vDir);
                if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                    CTransform* pTf = m_pOwner->Get_Transform();
                    _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                    _float fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                    _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt) - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                    _float fYaw = atan2f(fCross, fDot);
                    _float fStep = XMConvertToRadians(TURN_DEG) * dt;
                    _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                    pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                }
            }
            if (pAnim->Is_Finished()) { *bOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [bOn]() { *bOn = false; });

    return CBTSequence::Create({
        CBTSelector::Create({
            CBTCondition::Create([this](CBlackboard*) { return static_cast<CBoss_Armadillo*>(m_pOwner)->Has_Partner(); }),
            FaceWindup("AppearPartnerShort2", 360.f, SPD),
        }),
        pThrow,
        });
}

CBTNode* CBoss_Armadillo_Brain::Make_CatchOnce(shared_ptr<bool> pThrown)
{
    const _float fSwingRange = 6.f, fChaseTimeout = 5.f, fQTETime = 5.f;
    auto bWalk = make_shared<bool>(false); auto fWalkT = make_shared<_float>(0.f);
    auto* pWalkChase = CBTAction::Create(
        [this, bWalk, fWalkT, fSwingRange, fChaseTimeout](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bWalk) { m_pOwner->Get_BodyAnimator()->Play("Walk", true, true, 0.1f, SPD); m_pOwner->Get_Movement()->Set_MoveSpeed(CATCH_MOVE_SPEED); *fWalkT = 0.f; *bWalk = true; }
            *fWalkT += dt;
            _float fDist = pBB->Get<_float>("DistToTarget", FLT_MAX);
            if (fDist <= fSwingRange || *fWalkT >= fChaseTimeout) { *bWalk = false; return BT_STATUS::SUCCESS; }
            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vToTgt = XMLoadFloat3(&vDir);
            if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                m_pOwner->Add_MoveDir(vDir);
                CTransform* pTf = m_pOwner->Get_Transform();
                _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                _float fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt) - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                _float fYaw = atan2f(fCross, fDot);
                _float fStep = XMConvertToRadians(TURN_DEG) * dt;
                _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
            }
            return BT_STATUS::RUNNING;
        }, [bWalk]() { *bWalk = false; });

    auto bOn = make_shared<bool>(false);
    auto* pCatchRush = CBTAction::Create(
        [this, bOn](CBlackboard*, _float) -> BT_STATUS {
            auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
            CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
            if (!*bOn) { *bOn = true; pArma->Reset_CatchHit(); pAnim->Play("CatchAttack", false, true, 0.1f, SPD * 1.5f); pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, true); }
            if (pArma->Is_CatchHit()) pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false);
            if (pAnim->Is_Finished()) { pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false); *bOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [this, bOn]() { *bOn = false; static_cast<CBoss*>(m_pOwner)->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false); });

    auto bWaitOn = make_shared<bool>(false);
    auto* pQTEWait = CBTAction::Create(
        [this, bWaitOn, fQTETime](CBlackboard*, _float dt) -> BT_STATUS {
            auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
            if (!*bWaitOn) { *bWaitOn = true; pArma->Begin_QTE(fQTETime); m_pOwner->Get_BodyAnimator()->Play("CatchSuccessWait", true, true, 0.1f, SPD); }
            if (pArma->Is_QTEEscaped()) { *bWaitOn = false; pArma->End_QTE(); return BT_STATUS::SUCCESS; }
            if (pArma->Tick_QTETimer(dt)) { *bWaitOn = false; pArma->End_QTE(); return BT_STATUS::FAILURE; }
            return BT_STATUS::RUNNING;
        }, [this, bWaitOn]() { *bWaitOn = false; static_cast<CBoss_Armadillo*>(m_pOwner)->End_QTE(); });

    auto* pReleaseEscape = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Armadillo*>(m_pOwner)->Fire_Release(KIRBY_ATTACHMENT_END_REASON::GORILLA_COMBAT_ESCAPE);
        return BT_STATUS::SUCCESS; });

    auto bThrowOn = make_shared<bool>(false);
    auto* pThrow = CBTAction::Create(
        [this, bThrowOn](CBlackboard*, _float) -> BT_STATUS {
            CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
            if (!*bThrowOn) { *bThrowOn = true; pAnim->Play("CatchThrow", false, true, 0.1f, SPD); }
            if (pAnim->Is_Finished()) { *bThrowOn = false; static_cast<CBoss_Armadillo*>(m_pOwner)->Fire_Release(KIRBY_ATTACHMENT_END_REASON::GORILLA_COMBAT_THROWN); return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [bThrowOn]() { *bThrowOn = false; });

    return CBTSequence::Create({
        CBTAction::Create([pThrown](CBlackboard*, _float) { *pThrown = false; return BT_STATUS::SUCCESS; }),
        pWalkChase,
        FaceWindup("CatchAttackStart", 120.f, SPD * 2.f),
        pCatchRush,
        CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([this](CBlackboard*) { return static_cast<CBoss_Armadillo*>(m_pOwner)->Is_CatchHit(); }),
                Clip("CatchSuccess", SPD),
                CBTSelector::Create({
                    CBTSequence::Create({ pQTEWait, pReleaseEscape, Clip("CatchRelease", SPD) }),
                    CBTSequence::Create({ pThrow, CBTAction::Create([pThrown](CBlackboard*, _float) { *pThrown = true; return BT_STATUS::SUCCESS; }) }),
                }),
            }),
            Clip("CatchFailure", SPD),
        }),
        });
}

CBTNode* CBoss_Armadillo_Brain::Make_Catch()
{
    auto bThrown1 = make_shared<bool>(false);
    auto bThrown2 = make_shared<bool>(false);
    return CBTSequence::Create({
        CBTAction::Create([this](CBlackboard*, _float) { static_cast<CBoss_Armadillo*>(m_pOwner)->Show_Cage(); return BT_STATUS::SUCCESS; }),
        Clip("CarryStart", SPD),
        Make_CatchOnce(bThrown1),
        CBTSelector::Create({
            CBTCondition::Create([bThrown1](CBlackboard*) { return *bThrown1; }),
            CBTSequence::Create({
                Make_CatchOnce(bThrown2),
                CBTSelector::Create({
                    CBTCondition::Create([bThrown2](CBlackboard*) { return *bThrown2; }),
                    CBTSequence::Create({
                        Clip("CarryEnd", SPD),
                        CBTAction::Create([this](CBlackboard*, _float) { static_cast<CBoss_Armadillo*>(m_pOwner)->Hide_Cage(); return BT_STATUS::SUCCESS; }),
                    }),
                }),
            }),
        }),
        });
}

CBoss_Armadillo_Brain* CBoss_Armadillo_Brain::Create(CMonster* pOwner)
{
    CBoss_Armadillo_Brain* pInstance = new CBoss_Armadillo_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner)))
    {
        MSG_BOX("Failed to Created : CBoss_Armadillo_Brain"); Safe_Release(pInstance);
    }
    return pInstance;
}
void CBoss_Armadillo_Brain::Free() { __super::Free(); }