#include "Boss_Leopard_Brain.h"
#include "Boss_Leopard.h"
#include "BT.h"
#include "Monster_Movement.h"
#include "Boss_Leopard_Body.h"

CBTNode* CBoss_Leopard_Brain::Build_PhaseTree(_int)
{
    return CBTSequence::Create({
        Make_MountFirst(),
        Make_LeadIn(),
        Make_ChargeDash(),
        Make_GroundPhase(),
        Make_Remount(),
        });
}

const _char* CBoss_Leopard_Brain::IdleClip() const
{
    return static_cast<CBoss_Leopard*>(m_pOwner)->Is_PillarMode() ? "PillarWait" : "Wait";
}

CBTNode* CBoss_Leopard_Brain::Make_Turn()
{
    auto bTurn = make_shared<bool>(false);
    return CBTAction::Create(
        [this, bTurn](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bTurn) { Anim()->Play(IdleClip(), true, false, 0.2f, SPD); *bTurn = true; }
            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vT = XMLoadFloat3(&vDir);
            if (XMVector3Equal(vT, XMVectorZero())) { *bTurn = false; return BT_STATUS::SUCCESS; }
            if (IsFacing(vT, FACE_DOT)) { *bTurn = false; return BT_STATUS::SUCCESS; }
            RotateYawTo(vT, TURN_DEG, dt);
            return BT_STATUS::RUNNING;
        }, [bTurn] { *bTurn = false; });
}

CBTNode* CBoss_Leopard_Brain::Make_IdleHold(_float fHold)
{
    auto st = make_shared<pair<bool, _float>>(false, 0.f);
    return CBTAction::Create(
        [this, st, fHold](CBlackboard*, _float dt) -> BT_STATUS {
            if (!st->first) { Anim()->Play(IdleClip(), true, false, 0.2f, SPD); st->first = true; st->second = 0.f; }
            st->second += dt;
            if (st->second >= fHold) { st->first = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [st] { st->first = false; st->second = 0.f; });
}

CBTNode* CBoss_Leopard_Brain::Make_Throw()
{
    return CBTSequence::Create({ Clip("ThrowStart", SPD), Clip("Throw", SPD) });
}

CBTNode* CBoss_Leopard_Brain::Make_Arc(bool bAdvance, vector<string> airClips, _float fDur, _float fHeight)
{
    struct ARC { _float3 vStart{}, vEnd{}; _float t = 0.f; bool started = false; int idx = 0; };
    auto arc = make_shared<ARC>();
    return CBTAction::Create(
        [this, arc, fDur, fHeight, bAdvance, airClips](CBlackboard*, _float dt) -> BT_STATUS {
            auto* pLeo = static_cast<CBoss_Leopard*>(m_pOwner);
            CTransform* pTf = m_pOwner->Get_Transform(); auto* mv = m_pOwner->Get_Movement();
            CAnimator* pAnim = Anim();
            if (!arc->started) {
                pLeo->Enter_PillarMode();
                XMStoreFloat3(&arc->vStart, pTf->Get_State(STATE::POSITION));
                if (bAdvance) pLeo->Advance_ToAdjacentPillar();
                arc->vEnd = pLeo->Get_CurPillarPos();
                arc->t = 0.f; arc->started = true; arc->idx = 0;
                mv->Face_Instant(XMLoadFloat3(&arc->vEnd));
                if (!airClips.empty()) pAnim->Play(airClips[0], false, true, 0.1f, SPD);
            }
            if (pAnim->Is_Finished() && arc->idx + 1 < (int)airClips.size())
                pAnim->Play(airClips[++arc->idx], false, true, 0.1f, SPD);
            arc->t += dt; _float u = arc->t / fDur; if (u > 1.f) u = 1.f;
            _vector vPos = XMVectorLerp(XMLoadFloat3(&arc->vStart), XMLoadFloat3(&arc->vEnd), u);
            vPos = XMVectorSetY(vPos, XMVectorGetY(vPos) + 4.f * fHeight * u * (1.f - u));
            pTf->Set_State(STATE::POSITION, vPos); mv->Sync_To_Controller();
            if (u >= 1.f) {
                pTf->Set_State(STATE::POSITION, XMLoadFloat3(&arc->vEnd)); mv->Sync_To_Controller();
                mv->Face_Instant(XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos));
                arc->started = false; return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        }, [arc] { arc->started = false; arc->idx = 0; });
}

CBTNode* CBoss_Leopard_Brain::Make_AdjacentJump()
{
    return CBTSequence::Create({
        Clip("PillarJumpStart", SPD),
        Make_Arc(true, { "PillarJump", "PillarJumpRoll" }, 1.4f, 10.f),
        Clip("PillarLanding", SPD),
        });
}

CBTNode* CBoss_Leopard_Brain::Make_Remount()
{
    return CBTSequence::Create({
        Clip("PillarMoveStart", SPD),
        Make_Arc(false, { "PillarMove", "PillarJumpRoll" }, 1.6f, 12.f),
        Clip("PillarLanding", SPD),
        CBTAction::Create([this](CBlackboard*, _float) {
            static_cast<CBoss_Leopard*>(m_pOwner)->Spotlight_On_Snap();
            return BT_STATUS::SUCCESS; }),
        });
}

CBTNode* CBoss_Leopard_Brain::Make_MountFirst()
{
    auto bMounted = make_shared<bool>(false);
    return CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([bMounted](CBlackboard*) { return *bMounted; }),
            CBTAction::Create([](CBlackboard*, _float) { return BT_STATUS::SUCCESS; }),
        }),
        CBTSequence::Create({
            Make_Remount(),
            CBTAction::Create([bMounted](CBlackboard*, _float) {
                *bMounted = true;
                return BT_STATUS::SUCCESS; }),
        }),
        });
}

CBTNode* CBoss_Leopard_Brain::Make_LeadIn()
{
    auto bFirstCycle = make_shared<bool>(true);
    return CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([bFirstCycle](CBlackboard*) { return *bFirstCycle; }),
            Make_Turn(), Make_Throw(), Make_IdleHold(0.4f),
            Make_Turn(), Make_Throw(), Make_IdleHold(0.4f),
            CBTAction::Create([bFirstCycle](CBlackboard*, _float) { *bFirstCycle = false; return BT_STATUS::SUCCESS; }),
        }),
        CBTSequence::Create({
            Make_Turn(), Make_Throw(), Make_IdleHold(0.4f),
            Make_AdjacentJump(),
        }),
        });
}

CBTNode* CBoss_Leopard_Brain::Make_Dash()
{
    struct DASH { _float3 vDir{}; _float groundY = 0.f; _float t = 0.f; bool started = false; int idx = 0; };
    auto d = make_shared<DASH>();
    const _float fDashSpeed = 55.f, fMaxTime = 2.5f;
    const vector<string> clips = { "JumpAttackMove", "JumpAttack" };
    return CBTAction::Create(
        [this, d, fDashSpeed, fMaxTime, clips](CBlackboard*, _float dt) -> BT_STATUS {
            CTransform* pTf = m_pOwner->Get_Transform(); 
            CAnimator* pAnim = Anim(); 
            auto* mv = m_pOwner->Get_Movement();
            auto Leo = static_cast<CBoss_Leopard*>(m_pOwner);
            if (!d->started) {
                _vector vSelf = pTf->Get_State(STATE::POSITION);
                _float3 vTgt = m_pOwner->Get_BlackBoard().vTargetPos; d->groundY = vTgt.y;
                Leo->Spotlight_LockTarget(XMLoadFloat3(&vTgt));
                _vector vTo = XMVectorSubtract(XMLoadFloat3(&vTgt), vSelf);
                if (XMVectorGetX(XMVector3LengthSq(vTo)) < 1e-6f) vTo = pTf->Get_State(STATE::LOOK);
                XMStoreFloat3(&d->vDir, XMVector3Normalize(vTo));
                d->started = true; d->idx = 0; d->t = 0.f;
                mv->Face_Instant(XMLoadFloat3(&vTgt)); pAnim->Play(clips[0], false, true, 0.1f, SPD);
                Enable_Hit(CBoss_Leopard_Body::LHB_LCLAW, true);
                Enable_Hit(CBoss_Leopard_Body::LHB_RCLAW, true);
                Leo->Set_AfterimageFx(true, L"Afterimage_Jump");
            }
            if (pAnim->Is_Finished() && d->idx + 1 < (int)clips.size())
                pAnim->Play(clips[++d->idx], false, true, 0.1f, SPD);
            d->t += dt;
            _vector vPos = XMVectorAdd(pTf->Get_State(STATE::POSITION), XMLoadFloat3(&d->vDir) * fDashSpeed * dt);
            pTf->Set_State(STATE::POSITION, vPos); mv->Sync_To_Controller();
            if (XMVectorGetY(vPos) <= d->groundY || d->t >= fMaxTime) {
                _float3 p; XMStoreFloat3(&p, vPos); p.y = d->groundY;
                pTf->Set_State(STATE::POSITION, XMLoadFloat3(&p)); mv->Sync_To_Controller();
                Enable_Hit(CBoss_Leopard_Body::LHB_LCLAW, false);
                Enable_Hit(CBoss_Leopard_Body::LHB_RCLAW, false);
                Leo->Spawn_JumpSmoke(p);
                Leo->Set_AfterimageFx(false);
                Leo->Spawn_FloorFx();
                d->started = false; return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        }, [this, d] {
            if (!d->started) return;
            d->started = false;
            Enable_Hit(CBoss_Leopard_Body::LHB_LCLAW, false);
            Enable_Hit(CBoss_Leopard_Body::LHB_RCLAW, false);
            static_cast<CBoss_Leopard*>(m_pOwner)->Set_AfterimageFx(false);
            });
}

CBTNode* CBoss_Leopard_Brain::Make_Groggy()
{
    auto st = make_shared<pair<bool, _float>>(false, 0.f);
    const _float fGroggy = 3.f;
    return CBTAction::Create(
        [this, st, fGroggy](CBlackboard*, _float dt) -> BT_STATUS {
            if (!st->first) {
                static_cast<CBoss_Leopard*>(m_pOwner)->Exit_PillarMode();
                Anim()->Play("JumpAttackLeave", false, true, 0.1f, SPD);
                st->first = true; st->second = 0.f;
            }
            st->second += dt;
            if (st->second >= fGroggy) { 
                st->first = false; 
                static_cast<CBoss_Leopard*>(m_pOwner)->Spotlight_Off();
                return BT_STATUS::SUCCESS; 
            }
            return BT_STATUS::RUNNING;
        }, [st] { st->first = false; st->second = 0.f; });
}

CBTNode* CBoss_Leopard_Brain::Make_ChargeDash()
{
    return CBTSequence::Create({
        CBTAction::Create([this](CBlackboard*, _float) {
            static_cast<CBoss_Leopard*>(m_pOwner)->Spotlight_TrackKirby();
            return BT_STATUS::SUCCESS;
        }),
        FaceWindup("JumpAttackMoveStart", TURN_DEG, SPD),
        Make_Dash(), Make_Groggy(), Clip("JumpAttackEnd", SPD),
        });
}

CBTNode* CBoss_Leopard_Brain::Make_DropClaw()
{
    struct JMP { bool started = false; int idx = 0; bool fell = false; _float t = 0.f; };
    auto j = make_shared<JMP>();
    const _float fJumpDur = 1.5f, fJumpHeight = 10.f;
    const vector<string> up = { "DropClawJump", "DropClawJumpRollStart", "DropClawJumpRoll" };

    auto* pJump = CBTAction::Create(
        [this, j, fJumpDur, fJumpHeight, up](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement(); CAnimator* pAnim = Anim();
            if (!j->started) {
                _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                mv->Face_Instant(vTgt); mv->Begin_JumpArc(vTgt, fJumpDur, fJumpHeight);
                pAnim->Play(up[0], false, true, 0.1f, SPD);
                j->started = true; j->idx = 0; j->fell = false; j->t = 0.f;
            }
            j->t += dt;
            if (!j->fell) {
                if (pAnim->Is_Finished() && j->idx + 1 < (int)up.size())
                    pAnim->Play(up[++j->idx], false, true, 0.1f, SPD);
                _bool bRollDone = (j->idx >= (int)up.size() - 1) && pAnim->Is_Finished();
                if (bRollDone || j->t >= fJumpDur * 0.85f) {
                    pAnim->Play("DropClawFall", false, true, 0.1f, SPD);
                    Enable_Hit(CBoss_Leopard_Body::LHB_DROP, true);
                    j->fell = true;
                }
            }
            if (!mv->Is_JumpArc()) { j->started = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [this, j] { j->started = false; j->fell = false; Enable_Hit(CBoss_Leopard_Body::LHB_DROP, false); });

    auto bLand = make_shared<bool>(false);
    auto* pLanding = CBTAction::Create(
        [this, bLand](CBlackboard*, _float) -> BT_STATUS {
            CAnimator* pAnim = Anim();
            if (!*bLand) { Enable_Hit(CBoss_Leopard_Body::LHB_DROP, false); pAnim->Play("DropClawLanding", false, true, 0.1f, SPD); *bLand = true; }
            if (pAnim->Is_Finished()) { *bLand = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [bLand] { *bLand = false; });

    return CBTSequence::Create({ 
        FaceWindup("DropClawJumpStart", TURN_DEG, SPD),
        pJump,
        pLanding, });
}

CBTNode* CBoss_Leopard_Brain::Make_AssaultSlash()
{
    struct SL { _float3 vDir{}; bool started = false; _float prevSpd = 0.f; _float t = 0.f; };
    auto s = make_shared<SL>();
    const _float fDashSpeed = 55.f, fDashTime = 1.f;
    auto* pSlash = CBTAction::Create(
        [this, s, fDashSpeed, fDashTime](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement(); CAnimator* pAnim = Anim();
            auto  Leo = static_cast<CBoss_Leopard*>(m_pOwner);
            if (!s->started) {
                _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
                _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                _vector vTo = XMVectorSetY(XMVectorSubtract(vTgt, vSelf), 0.f);
                if (XMVectorGetX(XMVector3LengthSq(vTo)) < 1e-6f)
                    vTo = XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f);
                XMStoreFloat3(&s->vDir, XMVector3Normalize(vTo)); mv->Face_Instant(vTgt);
                s->prevSpd = mv->Get_MoveSpeed(); mv->Set_MoveSpeed(fDashSpeed);
                pAnim->Play("AssaultSlash", false, true, 0.1f, SPD);
                Enable_Hit(CBoss_Leopard_Body::LHB_ASSAULT, true);
                s->started = true; s->t = 0.f;
                Leo->Set_AfterimageFx(true, L"Afterimage_Assault");
            }
            m_pOwner->Add_MoveDir(s->vDir); s->t += dt;
            if (s->t >= fDashTime) {
                mv->Set_MoveSpeed(s->prevSpd);
                Enable_Hit(CBoss_Leopard_Body::LHB_ASSAULT, false);
                Leo->Spotlight_Off();
                Leo->Set_AfterimageFx(false);
                s->started = false; return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        }, [this, s] {
            if (!s->started)
                return;
            s->started = false;
            auto* mv = m_pOwner->Get_Movement();
            auto  Leo = static_cast<CBoss_Leopard*>(m_pOwner);
            if (mv && s->prevSpd > 0.f) mv->Set_MoveSpeed(s->prevSpd);
            Enable_Hit(CBoss_Leopard_Body::LHB_ASSAULT, false);
            Leo->Spotlight_Off();
            Leo->Set_AfterimageFx(false);
            });
        return CBTSequence::Create({
            CBTAction::Create([this](CBlackboard*, _float) {
                static_cast<CBoss_Leopard*>(m_pOwner)->Spotlight_On_Snap();
                return BT_STATUS::SUCCESS; }),
            FaceWindup("Ready", TURN_DEG, SPD),
            CBTAction::Create([this](CBlackboard*, _float) {
                static_cast<CBoss_Leopard*>(m_pOwner)->Spotlight_TrackKirby();
                return BT_STATUS::SUCCESS; }),
            FaceWindup("AssaultSlashStart", TURN_DEG, SPD),
            pSlash, Clip("AssaultSlashEnd", SPD),
            });
}

CBTNode* CBoss_Leopard_Brain::Make_GroundPattern()
{
    return CBTSelector::Create({
        CBTSequence::Create({ CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }), Make_DropClaw() }),
        Make_AssaultSlash(),
        });
}

CBTNode* CBoss_Leopard_Brain::Make_GroundPhase()
{
    return CBTSequence::Create({
        Make_GroundPattern(),
        CBTSelector::Create({
            CBTSequence::Create({ CBTCondition::Create([](CBlackboard*) { return (rand() & 1) == 0; }), Make_GroundPattern() }),
            CBTAction::Create([](CBlackboard*, _float) { return BT_STATUS::SUCCESS; }),
        }),
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