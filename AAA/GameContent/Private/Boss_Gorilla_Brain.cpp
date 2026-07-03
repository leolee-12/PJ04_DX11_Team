#include "Boss_Gorilla_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Animator.h"
#include "Monster_Movement.h"
#include "MultiHitBoxPart.h"
#include "Boss_Gorilla_Body.h"

CBTNode* CBoss_Gorilla_Brain::Build_PhaseTree(_int iPhase)
{
    const _float fCloseRange = 6.f;  
    const _float fSwingRange = 20.f;
    const _float fThrowRange = 40.f;
    const _float fFacingDot = 0.86f;
    const _float fTurnSpeedDeg = 90.f;
    const _float fChargeTime = 0.5f;
    const _float fSpd = 1.5f;
    const _float fAtkInterval = 2.5f;   
    const _float fStopRange = 5.f;


    // ---- 공유 헬퍼 ----
    auto Anim = [this]() -> CAnimator* { return m_pOwner->Get_BodyAnimator(); };
    auto OneShot = [&](const string& c) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, false, 0.f, fSpd });
        };
    auto HoldLoop = [&](const string& c, _float fHold) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, true, fHold, fSpd });
        };
    auto PlayFacing = [&](const string& c) -> CBTNode* {
        auto bP = make_shared<bool>(false);
        return CBTAction::Create(
            [this, bP, c, fSpd, fTurnSpeedDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bP) { pAnim->Play(c, false, true, 0.1f, fSpd); *bP = true; }

                _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                _vector vToTgt = XMLoadFloat3(&vDir);
                if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                    CTransform* pTf = m_pOwner->Get_Transform();
                    _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                    _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                    _float  fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                        - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                    _float  fYaw = atan2f(fCross, fDot);
                    _float  fStep = XMConvertToRadians(fTurnSpeedDeg) * dt;
                    _float  fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                    pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                }

                if (pAnim->Is_Finished()) { *bP = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [bP]() { *bP = false; });
        };
    auto HitBox = [this](_int idx, _bool on) -> CBTNode* {
        return CBTAction::Create([this, idx, on](CBlackboard*, _float) {
            if (auto* p = static_cast<CBoss*>(m_pOwner)->Get_HitBoxPart()) p->Enable_HitBox(idx, on);
            return BT_STATUS::SUCCESS; });
        };
    auto HitBoxAll = [this](_bool on) -> CBTNode* {
        return CBTAction::Create([this, on](CBlackboard*, _float) {
            if (auto* p = static_cast<CBoss*>(m_pOwner)->Get_HitBoxPart()) p->Enable_AllHitBoxes(on);
            return BT_STATUS::SUCCESS; });
        };

    // ---- 공격 빌더 ----
    auto MakeArmSwing = [&](_bool bRight) -> CBTNode* {
        const string s = bRight ? "R" : "L";
        const _int   idx = bRight ? CBoss_Gorilla_Body::GHB_RHAND : CBoss_Gorilla_Body::GHB_LHAND;
        return CBTSequence::Create({
            OneShot("ArmSwingChargeStart" + s),
            HoldLoop("ArmSwingChargeLoop" + s, fChargeTime),
            HitBox(idx, true),
            OneShot("ArmSwing" + s),
            HitBox(idx, false),
            });
        };

    auto MakeArmSpin = [&]() -> CBTNode* {
        const _float fSpinDuration = 5.f;     // 스핀 지속 시간
        const _float fSpinTurnDeg = 120.f;   // 스핀 중 타겟 추적 회전 속도(deg/s)
        auto fSpinT = make_shared<_float>(0.f);
        auto bSpin = make_shared<bool>(false);

        auto* pSpinChase = CBTAction::Create(
            [this, fSpinT, bSpin, fSpinDuration, fSpinTurnDeg, fSpd](CBlackboard* pBB, _float dt) -> BT_STATUS {
                if (!*bSpin) {
                    m_pOwner->Get_BodyAnimator()->Play("ArmSpin", true, true, 0.2f, fSpd);
                    *bSpin = true;
                }
                *fSpinT += dt;

                _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                _vector vToTgt = XMLoadFloat3(&vDir);
                if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                    m_pOwner->Add_MoveDir(vDir);
                    CTransform* pTf = m_pOwner->Get_Transform();
                    _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                    _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                    _float  fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                        - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                    _float  fYaw = atan2f(fCross, fDot);
                    _float  fStep = XMConvertToRadians(fSpinTurnDeg) * dt;
                    _float  fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                    pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                }

                if (*fSpinT >= fSpinDuration) { *fSpinT = 0.f; *bSpin = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [fSpinT, bSpin]() { *fSpinT = 0.f; *bSpin = false; });

        return CBTSequence::Create({
            OneShot("ArmSpinChargeStart"),
            HoldLoop("ArmSpinChargeLoop", fChargeTime),
            OneShot("ArmSpinStart"),
            HitBox(CBoss_Gorilla_Body::GHB_SPIN, true),
            pSpinChase,                                  // 기존 HoldLoop("ArmSpin", 5.f) 대체
            HitBox(CBoss_Gorilla_Body::GHB_SPIN, false),
            OneShot("ArmSpinEnd"),
            });
        };

    auto MakeStamp = [&](_bool bRight) -> CBTNode* {
        const string s = bRight ? "R" : "L";
        return CBTSequence::Create({
            OneShot("StampStart" + s),
            HoldLoop("StampWait" + s, fChargeTime),
            OneShot("Stamp" + s),
            });
        };

    // 타깃이 보스 기준 오른쪽이면 R, 왼쪽이면 L (진입 시 1회 판정)
    auto MakeStampSided = [&]() -> CBTNode* {
        return CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([this](CBlackboard* pBB) {
                    _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                    _vector vT = XMLoadFloat3(&vDir);
                    _vector vL = XMVector3Normalize(XMVectorSetY(
                        m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
                    _float fCross = XMVectorGetZ(vL) * XMVectorGetX(vT)
                                  - XMVectorGetX(vL) * XMVectorGetZ(vT);
                    return fCross >= 0.f;
                    }),
                MakeStamp(true),
                }),
            MakeStamp(false),
            });
        };

    // 찍기 패턴: 방향 찍기 2번 -> StampFinish(양발 마무리)
    auto MakeStampPattern = [&]() -> CBTNode* {
        return CBTSequence::Create({
            MakeStampSided(),
            MakeStampSided(),
            OneShot("StampFinish"),
            });
        };

    auto MakeThrowRock = [&]() -> CBTNode* {
        return CBTSequence::Create({
            PlayFacing("ThrowRockReady"),
            PlayFacing("ThrowRock"),
            });
        };

    auto MakeCatch = [&]() -> CBTNode* {
        return CBTSequence::Create({   // TODO: CatchAttack 적중 분기
            OneShot("CatchReadyStart"),
            OneShot("CatchReady"),
            OneShot("CatchAttack"),
            OneShot("CatchThrow"),
            });
        };

    auto MakeJumpToTarget = [&]() -> CBTNode* {
        auto bAir = make_shared<bool>(false);
        const _float fJumpTime = 1.5f;  
        const _float fJumpHeight = 10.f; 
        auto* pAir = CBTAction::Create(
            [this, bAir, fSpd, fJumpTime, fJumpHeight](CBlackboard*, _float) -> BT_STATUS {
                auto* mv = m_pOwner->Get_Movement();
                if (!*bAir) {
                    m_pOwner->Get_BodyAnimator()->Play("Jump", false, true, 0.1f, fSpd);

                    _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
                    _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);

                    _vector vToTgt = XMVectorSetY(vTgt - vSelf, 0.f);
                    if (XMVectorGetX(XMVector3LengthSq(vToTgt)) > 1e-6f)
                        vToTgt = XMVector3Normalize(vToTgt);
                    vTgt = vTgt - vToTgt * 5.f;

                    mv->Begin_JumpArc(vTgt, fJumpTime, fJumpHeight);
                    *bAir = true;
                }
                if (!mv->Is_JumpArc()) { *bAir = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [bAir]() { *bAir = false; });

        return CBTSequence::Create({
            OneShot("JumpStart"),
            pAir,
            OneShot("Landing"),
            OneShot("LandingEnd"),
            });
        };

    // 실행 시 두 노드 중 랜덤 (좌/우)
    auto Rand2 = [&](CBTNode* pA, CBTNode* pB) -> CBTNode* {
        return CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }),
                pA,
                }),
            pB,
            });
        };

    // 공격 1회당 카운터 +1 (밴드마다 새 노드)
    auto MeleeCount = [this]() -> CBTNode* {
        return CBTAction::Create([this](CBlackboard*, _float) {
            ++m_iMeleeSinceThrow; return BT_STATUS::SUCCESS; });
        };

    // ---- 공격 결정: 거리 사다리 ----
    CBTNode* pAttackDecision = CBTSelector::Create({
        // 1) 매 3공격: 점프(P0)/스핀(P1) -> 카운터 리셋
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard*) { return m_iMeleeSinceThrow >= 3; }),
            (iPhase == 0 ? MakeJumpToTarget() : MakeArmSpin()),
            CBTAction::Create([this](CBlackboard*, _float) {
                m_iMeleeSinceThrow = 0; return BT_STATUS::SUCCESS; }),
        }),
            // 2) 근접 (<= fCloseRange): 20% 잡기 / 80% 찍기패턴
        CBTSequence::Create({
            CBTCondition::Create([this, fCloseRange](CBlackboard* pBB) {
                return pBB->Get<_float>("DistToTarget", FLT_MAX) <= fCloseRange; }),
            CBTSelector::Create({
                CBTSequence::Create({
                    CBTCondition::Create([](CBlackboard*) { return (rand() % 100) < 20; }),
                    MakeCatch(),
                }),
                MakeStampPattern(),
            }),
            MeleeCount(),
        }),
        // 3) 중거리 (<= fSwingRange): 휘두르기 L/R
        CBTSequence::Create({
            CBTCondition::Create([this, fSwingRange](CBlackboard* pBB) {
                return pBB->Get<_float>("DistToTarget", FLT_MAX) <= fSwingRange; }),
            Rand2(MakeArmSwing(false), MakeArmSwing(true)),
            MeleeCount(),
        }),
            // 4) (P0 한정) 그 외 먼 거리: 돌던지기
        CBTSequence::Create({
            CBTCondition::Create([iPhase, fThrowRange](CBlackboard* pBB) {
                  return iPhase == 0 && pBB->Get<_float>("DistToTarget", FLT_MAX) >= fThrowRange; }),
            MakeThrowRock(),
            MeleeCount(),
        }),
    });

    // ---- 스캐폴딩 ----
    auto bMove = make_shared<bool>(false);
    auto* pChase = CBTAction::Create(
        [this, bMove, fSpd, fTurnSpeedDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bMove) { m_pOwner->Get_BodyAnimator()->Play("Walk", true, true, 0.2f, fSpd); *bMove = true; }

            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vToTgt = XMLoadFloat3(&vDir);
            CTransform* pTf = m_pOwner->Get_Transform();

            _float fDot = 1.f;
            if (!XMVector3Equal(vToTgt, XMVectorZero())) {           // 타겟 쪽으로 회전
                _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                    - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                _float fYaw = atan2f(fCross, fDot);
                _float fStep = XMConvertToRadians(fTurnSpeedDeg) * dt;
                _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
            }

            if (fDot > 0.3f) {                                       // 대충 정면일 때만 전진
                _vector vFwd = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                _float3 vMove; XMStoreFloat3(&vMove, vFwd);
                m_pOwner->Add_MoveDir(vMove);                        // 바라보는 방향으로 이동
            }
            return BT_STATUS::RUNNING;
        },
        [bMove]() { *bMove = false; });

    auto bHold = make_shared<bool>(false);
    auto* pHoldGround = CBTAction::Create(
        [this, bHold](CBlackboard*, _float) -> BT_STATUS {
            if (!*bHold) { m_pOwner->Get_BodyAnimator()->Play("Wait", true, true); *bHold = true; }
            return BT_STATUS::RUNNING;                                // 근접 시 회전 없이 그냥 대기
        },
        [bHold]() { *bHold = false; });

    auto* pFacing = CBTCondition::Create([this, fFacingDot](CBlackboard* pBB) {
        _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
        _vector vToTgt = XMLoadFloat3(&vDir);
        _vector vLook = XMVector3Normalize(XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
        return XMVectorGetX(XMVector3Dot(vLook, vToTgt)) >= fFacingDot;
        });

    auto bTurn = make_shared<bool>(false);
    auto* pTurnToTarget = CBTAction::Create(
        [this, bTurn, fTurnSpeedDeg, fFacingDot](CBlackboard* pBB, _float fDt) -> BT_STATUS {
            if (!*bTurn) { m_pOwner->Get_BodyAnimator()->Play("Wait", true, true); *bTurn = true; }

            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vToTgt = XMLoadFloat3(&vDir);
            if (XMVector3Equal(vToTgt, XMVectorZero())) { *bTurn = false; return BT_STATUS::SUCCESS; }

            CTransform* pTf = m_pOwner->Get_Transform();
            _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
            _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
            if (fDot >= fFacingDot) { *bTurn = false; return BT_STATUS::SUCCESS; }

            _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
            _float fYaw = atan2f(fCross, fDot);
            _float fStep = XMConvertToRadians(fTurnSpeedDeg) * fDt;
            _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
            pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
            return BT_STATUS::RUNNING;
        },
        [bTurn]() { *bTurn = false; });

    const _float fRecoverTime = 0.8f;
    auto fRecoverT = make_shared<_float>(0.f);
    auto* pPostAttackRecover = CBTAction::Create(
        [this, fRecoverT, fRecoverTime](CBlackboard*, _float dt) -> BT_STATUS {
            if (*fRecoverT == 0.f)
                m_pOwner->Get_BodyAnimator()->Play("Wait", true, true);   // 회전 없이 정지
            *fRecoverT += dt;
            if (*fRecoverT >= fRecoverTime) { *fRecoverT = 0.f; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [fRecoverT]() { *fRecoverT = 0.f; });


    // 공격 쿨다운: 매 프레임 dt 누적, 차면 SUCCESS / 아니면 FAILURE
    auto* pCooldownGate = CBTAction::Create([this, fAtkInterval](CBlackboard*, _float dt) -> BT_STATUS {
        m_fAtkTimer += dt;
        return (m_fAtkTimer >= fAtkInterval) ? BT_STATUS::SUCCESS : BT_STATUS::FAILURE;
        });
    auto* pResetTimer = CBTAction::Create([this](CBlackboard*, _float) {
        m_fAtkTimer = 0.f; return BT_STATUS::SUCCESS;
        });
    // 현재 거리에 맞는 공격이 있나? 없으면(애매한 거리) -> 공격 분기 실패 -> 추적
    auto* pAttackable = CBTCondition::Create([this, fSwingRange, fThrowRange, iPhase](CBlackboard* pBB) {
        if (m_iMeleeSinceThrow >= 3) return true;                  // 점프/스핀: 거리 무관
        _float d = pBB->Get<_float>("DistToTarget", FLT_MAX);
        if (d <= fSwingRange) return true;                         // 근접/휘두르기
        if (iPhase == 0 && d >= fThrowRange) return true;          // 던지기(P0)
        return false;                                              // 14~40 애매 -> 추적
        });

    auto bHeld = make_shared<bool>(false);   // 현재 정지 상태 유지 중인가
    auto* pStandStare = CBTSequence::Create({
       CBTCondition::Create([this, fStopRange, bHeld](CBlackboard* pBB) {
           _float d = pBB->Get<_float>("DistToTarget", FLT_MAX);
           _float fEnter = fStopRange;          // 5f 이내면 정지 시작
           _float fExit = fStopRange + 2.f;    // 7f 넘어야 정지 해제(추격 복귀)
           _bool bStay = *bHeld ? (d <= fExit) : (d <= fEnter);
           *bHeld = bStay;
           return bStay;
           }),
       pHoldGround,
        });

    // ---- 조립: 기본 추적, 쿨다운 차면 거리별 공격 ----
	CBTNode* pCombat = CBTReactiveSelector::Create({
		CBTSequence::Create({
			pCooldownGate,                       // N초 찼나? 안 찼으면 -> Chase
			CBTSequence::Create({
                pCooldownGate,
                pAttackable,
                CBTReactiveSelector::Create({
                    CBTReactiveSelector::Create({ 
                        CBTSequence::Create({ 
                            pFacing, 
                            pAttackDecision, 
                            pPostAttackRecover,
                            pResetTimer,
                            }),
				            pTurnToTarget,
				    }),
				}),
			}),
		}),
        pStandStare,
	    pChase,
	});

    return pCombat;
}

CBoss_Gorilla_Brain* CBoss_Gorilla_Brain::Create(CMonster* pOwner)
{
    CBoss_Gorilla_Brain* pInstance = new CBoss_Gorilla_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner)))
    {
        MSG_BOX("Failed to Created : CBoss_Gorilla_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Gorilla_Brain::Free()
{
    __super::Free();
}