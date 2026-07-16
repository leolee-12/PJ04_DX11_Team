#include "Boss_Leopard_Brain.h"
#include "Boss_Leopard.h"
#include "BT.h"
#include "Monster_Movement.h"
#include "Boss_Leopard_Body.h"

CBTNode* CBoss_Leopard_Brain::Build_PhaseTree(_int iPhase)
{
    UNREFERENCED_PARAMETER(iPhase);

    const _float fTurnSpeedDeg = 240.f;
    const _float fFacingDot = 0.96f;
    const _float fSpd = 1.5f;

    auto Anim = [this]() -> CAnimator* { return m_pOwner->Get_BodyAnimator(); };
    auto OneShot = [&](const string& c) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, false, 0.f, fSpd });
        };
    auto HoldLoop = [&](const string& c, _float fHold) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, true, fHold, fSpd });
        };
    auto MakeFaceWindup = [&](const string& clip) -> CBTNode* {
        auto bW = make_shared<bool>(false);
        return CBTAction::Create(
            [this, bW, clip, fSpd, fTurnSpeedDeg](CBlackboard*, _float dt) -> BT_STATUS {
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bW) { pAnim->Play(clip, false, true, 0.1f, fSpd); *bW = true; }

                CTransform* pTf = m_pOwner->Get_Transform();
                _vector vSelf = pTf->Get_State(STATE::POSITION);
                _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                _vector vTo = XMVectorSetY(XMVectorSubtract(vTgt, vSelf), 0.f);
                if (!XMVector3Equal(vTo, XMVectorZero())) {
                    vTo = XMVector3Normalize(vTo);
                    _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                    _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vTo));
                    _float  fCross = XMVectorGetZ(vLook) * XMVectorGetX(vTo) - XMVectorGetX(vLook) * XMVectorGetZ(vTo);
                    _float  fYaw = atan2f(fCross, fDot);
                    _float  fStep = XMConvertToRadians(fTurnSpeedDeg) * dt;
                    _float  fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                    pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                }

                if (pAnim->Is_Finished()) { *bW = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [bW]() { *bW = false; });
        };

    // ---- 커비를 향해 부드럽게 회전 ----
    auto MakeTurn = [&]() -> CBTNode* {
        auto bTurn = make_shared<bool>(false);
        return CBTAction::Create(
            [this, bTurn, fTurnSpeedDeg, fFacingDot, fSpd](CBlackboard* pBB, _float dt) -> BT_STATUS {
                if (!*bTurn) {
                    const char* idle = static_cast<CBoss_Leopard*>(m_pOwner)->Is_PillarMode() ? "PillarWait" : "Wait";
                    m_pOwner->Get_BodyAnimator()->Play(idle, true, false, 0.2f, fSpd);
                    *bTurn = true;
                }
                _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                _vector vT = XMLoadFloat3(&vDir);
                if (XMVector3Equal(vT, XMVectorZero())) { *bTurn = false; return BT_STATUS::SUCCESS; }
                CTransform* pTf = m_pOwner->Get_Transform();
                _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vT));
                if (fDot >= fFacingDot) { *bTurn = false; return BT_STATUS::SUCCESS; }
                _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vT) - XMVectorGetX(vLook) * XMVectorGetZ(vT);
                _float fYaw = atan2f(fCross, fDot);
                _float fStep = XMConvertToRadians(fTurnSpeedDeg) * dt;
                _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                return BT_STATUS::RUNNING;
            },
            [bTurn]() { *bTurn = false; });
        };

    // ---- 손톱 던지기 ----
    auto MakeThrow = [&]() -> CBTNode* {
        return CBTSequence::Create({
            OneShot("ThrowStart"),
            OneShot("Throw"),
            });
        };

    // ---- 아크 도약 코어: 공중 클립들을 순차 체인하며 아크(위치) 이동 ----
    // airClips[0] 재생 -> 끝나면 다음 클립 -> ... 마지막 클립은 홀드(재시작 안 함).
    // 완료 판정은 '아크 도착'(u>=1)만 본다.
    auto MakeArc = [&](bool bAdvance, vector<string> airClips, _float fDur, _float fHeight) -> CBTNode* {
        struct ARC { _float3 vStart{}, vEnd{}; _float t = 0.f; bool started = false; int idx = 0; };
        auto arc = make_shared<ARC>();

        return CBTAction::Create(
            [this, arc, fDur, fHeight, fSpd, bAdvance, airClips](CBlackboard*, _float dt) -> BT_STATUS {
                auto* pLeo = static_cast<CBoss_Leopard*>(m_pOwner);
                CTransform* pTf = m_pOwner->Get_Transform();
                auto* mv = m_pOwner->Get_Movement();
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();

                if (!arc->started) {
                    pLeo->Enter_PillarMode();                    // CCT off + 중력0 (스냅 없음)
                    XMStoreFloat3(&arc->vStart, pTf->Get_State(STATE::POSITION));
                    if (bAdvance) pLeo->Advance_ToAdjacentPillar();
                    arc->vEnd = pLeo->Get_CurPillarPos();
                    arc->t = 0.f; arc->started = true; arc->idx = 0;

                    mv->Face_Instant(XMLoadFloat3(&arc->vEnd));
                    if (!airClips.empty())
                        pAnim->Play(airClips[0], false, true, 0.1f, fSpd);
                }

                // 공중 클립 체인: 현재 클립 끝났고 다음이 있으면 넘어감 (마지막은 홀드)
                if (pAnim->Is_Finished() && arc->idx + 1 < (int)airClips.size())
                {
                    ++arc->idx;
                    pAnim->Play(airClips[arc->idx], false, true, 0.1f, fSpd);
                }

                arc->t += dt;
                _float u = arc->t / fDur; if (u > 1.f) u = 1.f;

                _vector vPos = XMVectorLerp(XMLoadFloat3(&arc->vStart), XMLoadFloat3(&arc->vEnd), u);
                _float  fHop = 4.f * fHeight * u * (1.f - u);
                vPos = XMVectorSetY(vPos, XMVectorGetY(vPos) + fHop);
                pTf->Set_State(STATE::POSITION, vPos);
                mv->Sync_To_Controller();

                if (u >= 1.f) {
                    pTf->Set_State(STATE::POSITION, XMLoadFloat3(&arc->vEnd));
                    mv->Sync_To_Controller();
                    mv->Face_Instant(XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos));
                    arc->started = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [arc]() { arc->started = false; arc->idx = 0; });
        };

    // ---- 인접 기둥 점프: PillarJumpStart -> (PillarJump -> PillarJumpRoll + 아크) -> 착지 ----
    auto MakeAdjacentJump = [&]() -> CBTNode* {
        return CBTSequence::Create({
            OneShot("PillarJumpStart"),                                   
            MakeArc(true, { "PillarJump", "PillarJumpRoll" }, 1.4f, 10.f),
            OneShot("PillarLanding"),                                     
            });
        };

    auto MakeRemount = [&]() -> CBTNode* {
        return CBTSequence::Create({
            OneShot("PillarMoveStart"),
            MakeArc(false, { "PillarMove", "PillarJumpRoll" }, 1.6f, 12.f),
            OneShot("PillarLanding"),
            });
        };

    // ---- 첫 마운트는 1회만 ----
    auto bMounted = make_shared<bool>(false);
    auto* pMountFirst = CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([bMounted](CBlackboard*) { return *bMounted; }),
            CBTAction::Create([](CBlackboard*, _float) { return BT_STATUS::SUCCESS; }),
            }),
        CBTSequence::Create({
            MakeRemount(),
            CBTAction::Create([bMounted](CBlackboard*, _float) { *bMounted = true; return BT_STATUS::SUCCESS; }),
            }),
        });

    /*auto iThrowCnt = make_shared<int>(0);
    auto* pMaybeJump = CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([iThrowCnt](CBlackboard*) { return (++(*iThrowCnt) % 2) == 0; }),
            MakeAdjacentJump(),
            }),
        CBTAction::Create([](CBlackboard*, _float) { return BT_STATUS::SUCCESS; }),
        });*/

    auto MakeIdleHold = [&](_float fHold) -> CBTNode* {
        auto st = make_shared<pair<bool, _float>>(false, 0.f);
        return CBTAction::Create(
            [this, st, fHold, fSpd](CBlackboard*, _float dt) -> BT_STATUS {
                if (!st->first) {
                    const char* idle = static_cast<CBoss_Leopard*>(m_pOwner)->Is_PillarMode()
                        ? "PillarWait" : "Wait";
                    m_pOwner->Get_BodyAnimator()->Play(idle, true, false, 0.2f, fSpd);
                    st->first = true; st->second = 0.f;
                }
                st->second += dt;
                if (st->second >= fHold) { st->first = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [st]() { st->first = false; st->second = 0.f; });
        };

    // ---- 커비 향해 일직선 돌진 (Move->Attack 체인), 땅 닿으면 종료 ----
    auto MakeDash = [&]() -> CBTNode* {
        struct DASH { _float3 vDir{}; _float groundY = 0.f; _float t = 0.f; bool started = false; int idx = 0; };
        auto d = make_shared<DASH>();
        const _float fDashSpeed = 55.f;   // 돌진 속도
        const _float fMaxTime = 2.5f;   // 안전 타임아웃(지면 못 만나는 예외)
        const vector<string> clips = { "JumpAttackMove", "JumpAttack" };

        return CBTAction::Create(
            [this, d, fDashSpeed, fMaxTime, clips, fSpd](CBlackboard*, _float dt) -> BT_STATUS {
                CTransform* pTf = m_pOwner->Get_Transform();
                auto* mv = m_pOwner->Get_Movement();
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();

                if (!d->started) {
                    _vector vSelf = pTf->Get_State(STATE::POSITION);
                    _float3 vTgt = m_pOwner->Get_BlackBoard().vTargetPos;
                    d->groundY = vTgt.y;                          // 커비 발밑 ~= 지면
                    _vector vTo = XMVectorSubtract(XMLoadFloat3(&vTgt), vSelf);
                    if (XMVectorGetX(XMVector3LengthSq(vTo)) < 1e-6f)
                        vTo = pTf->Get_State(STATE::LOOK);
                    XMStoreFloat3(&d->vDir, XMVector3Normalize(vTo));   // 3D 일직선 방향(아래로 향함)
                    d->started = true; d->idx = 0; d->t = 0.f;

                    mv->Face_Instant(XMLoadFloat3(&vTgt));
                    pAnim->Play(clips[0], false, true, 0.1f, fSpd);
                    if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart()) {
                        p->Enable_HitBox(CBoss_Leopard_Body::LHB_LCLAW, true);
                        p->Enable_HitBox(CBoss_Leopard_Body::LHB_RCLAW, true);
                    }
                }

                if (pAnim->Is_Finished() && d->idx + 1 < (int)clips.size()) {
                    ++d->idx;
                    pAnim->Play(clips[d->idx], false, true, 0.1f, fSpd);
                }

                d->t += dt;
                _vector vPos = pTf->Get_State(STATE::POSITION);
                vPos = XMVectorAdd(vPos, XMLoadFloat3(&d->vDir) * fDashSpeed * dt);
                pTf->Set_State(STATE::POSITION, vPos);
                mv->Sync_To_Controller();

                if (XMVectorGetY(vPos) <= d->groundY || d->t >= fMaxTime) {
                    _float3 p; XMStoreFloat3(&p, vPos); p.y = d->groundY;
                    pTf->Set_State(STATE::POSITION, XMLoadFloat3(&p));
                    mv->Sync_To_Controller();
                    if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart()) {
                        p->Enable_HitBox(CBoss_Leopard_Body::LHB_LCLAW, false);
                        p->Enable_HitBox(CBoss_Leopard_Body::LHB_RCLAW, false);
                    }
                    d->started = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [this, d]() {
                d->started = false;
                if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart()) {
                    p->Enable_HitBox(CBoss_Leopard_Body::LHB_LCLAW, false);
                    p->Enable_HitBox(CBoss_Leopard_Body::LHB_RCLAW, false);
                }
            });
        };

    auto MakeGroggy = [&]() -> CBTNode* {
        auto st = make_shared<pair<bool, _float>>(false, 0.f);
        const _float fGroggy = 3.f;
        return CBTAction::Create(
            [this, st, fGroggy, fSpd](CBlackboard*, _float dt) -> BT_STATUS {
                if (!st->first) {
                    static_cast<CBoss_Leopard*>(m_pOwner)->Exit_PillarMode();   // 땅 공격 시점 = CCT on + 중력복원
                    m_pOwner->Get_BodyAnimator()->Play("JumpAttackLeave", false, true, 0.1f, fSpd);
                    // TODO: 손톱 땅에 꽂히는 FX/SFX, 카메라 셰이크
                    st->first = true; st->second = 0.f;
                }
                st->second += dt;
                if (st->second >= fGroggy) { st->first = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;   // 클립 끝나도 홀드(재시작 X), 3초까지 그로기 유지
            },
            [st]() { st->first = false; st->second = 0.f; });
        };

    auto MakeChargeDash = [&]() -> CBTNode* {
        return CBTSequence::Create({
            MakeFaceWindup("JumpAttackMoveStart"),
            MakeDash(),                        
            MakeGroggy(),                      
            OneShot("JumpAttackEnd"),          
            });
        };

    auto MakeDropClaw = [&]() -> CBTNode* {
        struct JMP { bool started = false; int idx = 0; bool fell = false; _float t = 0.f; };
        auto j = make_shared<JMP>();
        const _float fJumpDur = 1.5f;
        const _float fJumpHeight = 10.f;
        const vector<string> up = { "DropClawJump", "DropClawJumpRollStart", "DropClawJumpRoll" };

        auto* pJump = CBTAction::Create(
            [this, j, fJumpDur, fJumpHeight, up, fSpd](CBlackboard*, _float dt) -> BT_STATUS {
                auto* mv = m_pOwner->Get_Movement();
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();

                if (!j->started) {
                    _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                    mv->Face_Instant(vTgt);
                    mv->Begin_JumpArc(vTgt, fJumpDur, fJumpHeight);
                    pAnim->Play(up[0], false, true, 0.1f, fSpd);
                    j->started = true; j->idx = 0; j->fell = false; j->t = 0.f;
                }
                j->t += dt;

                if (!j->fell)
                {
                    if (pAnim->Is_Finished() && j->idx + 1 < (int)up.size()) {
                        ++j->idx;
                        pAnim->Play(up[j->idx], false, true, 0.1f, fSpd);
                    }

                    _bool bRollDone = (j->idx >= (int)up.size() - 1) && pAnim->Is_Finished();
                    _bool bArcEnding = (j->t >= fJumpDur * 0.85f);
                    if (bRollDone || bArcEnding) {
                        pAnim->Play("DropClawFall", false, true, 0.1f, fSpd);
                        if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart())
                            p->Enable_HitBox(CBoss_Leopard_Body::LHB_DROP, true);
                        j->fell = true;
                    }
                }

                if (!mv->Is_JumpArc()) {
                    j->started = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [this, j]() {
                j->started = false; j->fell = false;
                if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart())
                    p->Enable_HitBox(CBoss_Leopard_Body::LHB_DROP, false);
            });

        auto bLand = make_shared<bool>(false);
        auto* pLanding = CBTAction::Create(
            [this, bLand, fSpd](CBlackboard*, _float) -> BT_STATUS {
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bLand) {
                    if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart())
                        p->Enable_HitBox(CBoss_Leopard_Body::LHB_DROP, false);
                    pAnim->Play("DropClawLanding", false, true, 0.1f, fSpd);
                    *bLand = true;
                }
                if (pAnim->Is_Finished()) { *bLand = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [bLand]() { *bLand = false; });

        return CBTSequence::Create({
            OneShot("DropClawJumpStart"),    
            pJump,                           
            pLanding,                        
            });
        };

    auto MakeAssaultSlash = [&]() -> CBTNode* {
        struct SL { _float3 vDir{}; bool started = false; _float prevSpd = 0.f; _float t = 0.f;};
        auto s = make_shared<SL>();
        const _float fDashSpeed = 30.f;  
        const _float fDashTime = 1.5f;

        auto* pSlash = CBTAction::Create(
            [this, s, fDashSpeed, fDashTime, fSpd](CBlackboard*, _float dt) -> BT_STATUS {
                auto* mv = m_pOwner->Get_Movement();
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();

                if (!s->started) {
                    _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
                    _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                    _vector vTo = XMVectorSetY(XMVectorSubtract(vTgt, vSelf), 0.f);
                    if (XMVectorGetX(XMVector3LengthSq(vTo)) < 1e-6f)
                        vTo = XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f);
                    XMStoreFloat3(&s->vDir, XMVector3Normalize(vTo));
                    mv->Face_Instant(vTgt);

                    s->prevSpd = mv->Get_MoveSpeed();
                    mv->Set_MoveSpeed(fDashSpeed);             
                    pAnim->Play("AssaultSlash", false, true, 0.1f, fSpd);

                    if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart()) {
                        p->Enable_HitBox(CBoss_Leopard_Body::LHB_ASSAULT, true);
                    }
                    s->started = true;
                    s->t = 0.f;
                }

                m_pOwner->Add_MoveDir(s->vDir);
                s->t += dt;

                if (s->t >= fDashTime) {
                    mv->Set_MoveSpeed(s->prevSpd);
                    if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart()) {
                        p->Enable_HitBox(CBoss_Leopard_Body::LHB_ASSAULT, false);
                    }
                    s->started = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [this, s]() {
                s->started = false;
                auto* mv = m_pOwner->Get_Movement();
                if (mv && s->prevSpd > 0.f) mv->Set_MoveSpeed(s->prevSpd);
                if (auto* p = static_cast<CBoss_Leopard*>(m_pOwner)->Get_HitBoxPart()) {
                    p->Enable_HitBox(CBoss_Leopard_Body::LHB_ASSAULT, false);
                }
            });

        return CBTSequence::Create({
            MakeFaceWindup("Ready"),
            MakeFaceWindup("AssaultSlashStart"),
            pSlash,                         
            OneShot("AssaultSlashEnd"),     
            });
        };

    auto MakeGroundPattern = [&]() -> CBTNode* {
        return CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }),
                MakeDropClaw(),
                }),
            MakeAssaultSlash(),
            });
        };

    auto MakeGroundPhase = [&]() -> CBTNode* {
        return CBTSequence::Create({
            MakeGroundPattern(),
            CBTSelector::Create({
                CBTSequence::Create({
                    CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }),
                    MakeGroundPattern(),
                    }),
                CBTAction::Create([](CBlackboard*, _float) { return BT_STATUS::SUCCESS; }),
                }),
            });
        };

    // ---- 루트 ----
    return CBTSequence::Create({
        pMountFirst,
        MakeTurn(), MakeThrow(), MakeIdleHold(0.4f),
        MakeTurn(), MakeThrow(), MakeIdleHold(0.4f),
        MakeChargeDash(),                           
        MakeGroundPhase(),                          
        MakeRemount(),                              
        });
}

CBoss_Leopard_Brain* CBoss_Leopard_Brain::Create(CMonster* pOwner)
{
    CBoss_Leopard_Brain* pInstance = new CBoss_Leopard_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner)))
    {
        MSG_BOX("Failed to Created : CBoss_Leopard_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Leopard_Brain::Free() { __super::Free(); }