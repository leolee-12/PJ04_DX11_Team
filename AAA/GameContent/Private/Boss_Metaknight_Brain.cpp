#include "Boss_Metaknight_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Animator.h"
#include "Monster_Movement.h"

namespace
{
    inline _float Saturate(_float f)
    {
        return f < 0.f ? 0.f : (f > 1.f ? 1.f : f);
    }

    inline _float SmoothStep01(_float t)
    {
        t = Saturate(t);
        return t * t * (3.f - 2.f * t);
    }
}

CBTNode* CBoss_Metaknight_Brain::Build_PhaseTree(_int iPhase)
{
    // 튜닝용
    //return CBTReactiveSelector::Create({
    //    Make_UpperBranch(),
    //    Loop("Wait", 0.5f, SPD),
    //    });
    
    // 에디터 애니매이션 편집용
    return CBTReactiveSelector::Create({
        Clip("Wait", SPD, 0.f),
        });

    CBTNode* pCombat = CBTSequence::Create({
        Make_GigaBranch(),
        Make_StepApproach(),
        Make_UnlessInRange(Make_DashIn()),
        Make_ComboBranch(),
        });

    if (iPhase >= 1)
    {
        return CBTReactiveSelector::Create({
            Make_LockOutcomeBranch(),
            Make_DodgeBranch(),
            Make_RockBranch(),
            Make_UpperBranch(),
            pCombat,
            });
    }
    return CBTReactiveSelector::Create({
        Make_LockOutcomeBranch(),
        Make_DodgeBranch(),
        Make_RockBranch(),
        Make_UpperBranch(),
        pCombat,
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_Optional(CBTNode* pCond, CBTNode* pBody)
{
    return CBTSelector::Create({
        CBTSequence::Create({ pCond, pBody }),
        CBTCondition::Create([](CBlackboard*) { return true; }),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UnlessInRange(CBTNode* pNode)
{
    return Make_Optional(
        CBTCondition::Create([](CBlackboard* pBB) {
            return pBB->Get<_float>("DistToTarget", FLT_MAX) > COMBO_RANGE; }),
            pNode);
}

CBTNode* CBoss_Metaknight_Brain::Make_Step()
{
    auto bOn = make_shared<bool>(false);

    return CBTAction::Create(
        [this, bOn](CBlackboard*, _float dt) -> BT_STATUS {
            if (!*bOn)
            {
                Anim()->Play("FrontMove", false, true, 0.1f, SPD);
                *bOn = true;
            }

            _vector vDir = Dir_ToTargetXZ();
            RotateYawTo(vDir, TURN_DEG, dt);

            m_pOwner->Get_Movement()->Set_WindowMoveSpeed(STEP_SPEED, Anim()->Get_Progress());
            m_pOwner->Add_MoveDir(m_pOwner->Get_Transform()->Get_State(STATE::LOOK));

            if (Anim()->Is_Finished())
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bOn] { *bOn = false; });
}

CBTNode* CBoss_Metaknight_Brain::Make_StepApproach()
{
    auto* pRoll = CBTAction::Create([](CBlackboard* pBB, _float) {
        pBB->Set<_int>("StepCount", 1 + rand() % 3);
        return BT_STATUS::SUCCESS;
        });

    auto Optional = [this](_int n) -> CBTNode* {
        return Make_Optional(
            CBTCondition::Create([n](CBlackboard* pBB) {
                return pBB->Get<_int>("StepCount", 1) >= n; }),
                CBTSequence::Create({ Loop("Wait", STEP_PAUSE, SPD), Make_RandStep() }));
        };

    return CBTSequence::Create({
        pRoll,
        Make_RandStep(),
        Optional(2),
        Optional(3),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_DashIn()
{
    auto bOn = make_shared<bool>(false);
    auto fElapsed = make_shared<_float>(0.f);
    auto vLockPos = make_shared<_float3>();
    auto vLockDir = make_shared<_float3>();

    return CBTAction::Create(
        [this, bOn, fElapsed, vLockPos, vLockDir](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bOn)
            {
                _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
                _vector vTarget = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);

                _vector vDir = XMVectorSetY(vTarget - vSelf, 0.f);
                const _float fDist = XMVectorGetX(XMVector3Length(vDir));

                if (fDist <= DASH_STOP_DIST + DASH_ARRIVE_DIST)
                    return BT_STATUS::SUCCESS;

                vDir = XMVector3Normalize(vDir);

                XMStoreFloat3(vLockPos.get(), vTarget - vDir * DASH_STOP_DIST);
                XMStoreFloat3(vLockDir.get(), vDir);

                m_pOwner->Get_Movement()->Face_Instant(vTarget);

                Anim()->Play("Dash", true, true, 0.1f, SPD);
                *fElapsed = 0.f;
                *bOn = true;
            }

            *fElapsed += dt;

            m_pOwner->Get_Movement()->Set_MoveSpeed(DASH_SPEED);
            m_pOwner->Add_MoveDir(XMLoadFloat3(vLockDir.get()));

            _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
            _vector vToPoint = XMVectorSetY(XMLoadFloat3(vLockPos.get()) - vSelf, 0.f);

            const _float fRemain = XMVectorGetX(XMVector3Length(vToPoint));
            const _bool  bPassed = XMVectorGetX(XMVector3Dot(vToPoint, XMLoadFloat3(vLockDir.get()))) < 0.f;

            if (fRemain <= DASH_ARRIVE_DIST || bPassed || *fElapsed >= DASH_TIMEOUT)
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bOn, fElapsed]
        {
            *bOn = false;
            *fElapsed = 0.f;
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_SideStep(_bool bRight)
{
    auto bOn = make_shared<bool>(false);
    auto fRadius = make_shared<_float>(0.f);

    return CBTAction::Create(
        [this, bOn, bRight, fRadius](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            CTransform* pTf = m_pOwner->Get_Transform();

            if (!*bOn)
            {
                Anim()->Play(bRight ? "RightSideMove" : "LeftSideMove", false, true, 0.1f, SPD);
                mv->Set_LockFacing(true);

                _vector vSelf = pTf->Get_State(STATE::POSITION);
                _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                *fRadius = XMVectorGetX(XMVector3Length(XMVectorSetY(vTgt - vSelf, 0.f)));
                *bOn = true;
            }

            RotateYawTo(Dir_ToTargetXZ(), TURN_DEG, dt);

            _vector vSelf = pTf->Get_State(STATE::POSITION);
            _vector vTgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
            _vector vToTgt = XMVectorSetY(vTgt - vSelf, 0.f);
            _float fDist = XMVectorGetX(XMVector3Length(vToTgt));
            if (fDist < 1e-3f) fDist = 1e-3f;
            vToTgt /= fDist;

            _vector vTan = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vToTgt);
            if (!bRight) vTan = -vTan;

            _vector vMove = XMVector3Normalize(vTan + vToTgt * (fDist - *fRadius) * RADIUS_GAIN);

            mv->Set_WindowMoveSpeed(SIDE_SPEED, Anim()->Get_Progress());
            m_pOwner->Add_MoveDir(vMove);

            if (Anim()->Is_Finished())
            {
                mv->Set_LockFacing(false);
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn] {
            *bOn = false;
            m_pOwner->Get_Movement()->Set_LockFacing(false);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_RandStep()
{
    auto* pRoll = CBTAction::Create([](CBlackboard* pBB, _float) {
        pBB->Set<_int>("StepPick", rand() % 3);
        return BT_STATUS::SUCCESS;
        });

    auto Branch = [](_int n, CBTNode* pStep) -> CBTNode* {
        return CBTSequence::Create({
            CBTCondition::Create([n](CBlackboard* pBB) {
                return pBB->Get<_int>("StepPick", 0) == n; }),
            pStep,
            });
        };

    return CBTSequence::Create({ pRoll,
        CBTSelector::Create({
            Branch(0, Make_Step()),
            Branch(1, Make_SideStep(false)),
            Branch(2, Make_SideStep(true)),
        }),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_Dodge()
{
    auto bOn = make_shared<bool>(false);
    auto fPrevP = make_shared<_float>(0.f);
    auto vSide = make_shared<_float3>();
    auto bRight = make_shared<bool>(false);

    return CBTAction::Create(
        [this, bOn, fPrevP, vSide, bRight](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            auto* pBoss = static_cast<CBoss_Metaknight*>(m_pOwner);
            if (!*bOn)
            {
                Anim()->Play("Dodge", false, true, 0.05f, SPD);
                mv->Set_LockFacing(true);
                pBoss->Set_DodgeInvincible(true);

                *bRight = (rand() & 1) != 0;
                _vector vTan = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), Dir_ToTargetXZ());
                if (!*bRight) vTan = -vTan;
                XMStoreFloat3(vSide.get(), vTan);

                *fPrevP = 0.f;
                *bOn = true;
            }

            const _float p = Anim()->Get_Progress();
            const _float fFrac = max(0.f, p - *fPrevP);
            *fPrevP = p;

            _float fRad = XMConvertToRadians(360.f * fFrac);
            if (!*bRight) fRad = -fRad;
            m_pOwner->Get_Transform()->Rotate(
                XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fRad));

            if (dt > 1e-6f && fFrac > 0.f)
            {
                mv->Set_MoveSpeed(DODGE_DIST * fFrac / dt);
                m_pOwner->Add_MoveDir(XMLoadFloat3(vSide.get()));
            }

            if (Anim()->Is_Finished())
            {
                pBoss->Set_DodgeInvincible(false);
                mv->Set_LockFacing(false);
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn, fPrevP] {
            *bOn = false; *fPrevP = 0.f;
            static_cast<CBoss_Metaknight*>(m_pOwner)->Set_DodgeInvincible(false);
            m_pOwner->Get_Movement()->Set_LockFacing(false);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_SwordHit(_bool bOn)
{
    return CBTAction::Create([this, bOn](CBlackboard*, _float) {
        static_cast<CBoss_Metaknight*>(m_pOwner)->Enable_SwordHit(bOn);
        return BT_STATUS::SUCCESS;
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_SwordCombo(_int iHits)
{
    CBTNode* pOpen = CBTSequence::Create({
        //FaceWindup("Attack1Start", 540.f, SPD),
        Clip("Attack1Charge", 2.f, 0.2f),
        Make_SwordHit(true),
        Make_AttackLunge("Attack1"),
        });

    switch (iHits)
    {
        case 1:
            return CBTSequence::Create({ pOpen,
                Make_SwordHit(false), Clip("Attack1End", SPD) });
        case 2:
            return CBTSequence::Create({ pOpen,
                Make_AttackLunge("Attack2"),
                Make_SwordHit(false), Clip("Attack2End", SPD) });
        default:
            return CBTSequence::Create({ pOpen,
                Make_AttackLunge("Attack2"),
                Make_AttackLunge("Attack3"),
                Make_SwordHit(false), Clip("Attack3End", SPD) });
    }
}

CBTNode* CBoss_Metaknight_Brain::Make_AttackLunge(const string& strClip)
{
    auto bOn = make_shared<bool>(false);

    return CBTAction::Create(
        [this, bOn, strClip](CBlackboard*, _float) -> BT_STATUS {
            if (!*bOn)
            {
                Anim()->Play(strClip.c_str(), false, true, 0.2f, SPD);
                *bOn = true;
            }

            m_pOwner->Get_Movement()->Set_WindowMoveSpeed(ATK_LUNGE_SPEED, Anim()->Get_Progress());
            m_pOwner->Add_MoveDir(m_pOwner->Get_Transform()->Get_State(STATE::LOOK));

            if (Anim()->Is_Finished())
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bOn] { *bOn = false; });
}

CBTNode* CBoss_Metaknight_Brain::Make_ComboPick()
{
    auto* pRoll = CBTAction::Create(
        [this](CBlackboard* pBB, _float) {
            static_cast<CBoss_Metaknight*>(m_pOwner)->Set_AttackBusy(true);
            _int iPick;
            do { iPick = 1 + rand() % 3; } while (iPick == m_iLastCombo);
            m_iLastCombo = iPick;
            pBB->Set<_int>("ComboPick", iPick);
            return BT_STATUS::SUCCESS;
        },
        [this] { static_cast<CBoss_Metaknight*>(m_pOwner)->Set_AttackBusy(false); });

    auto Branch = [this](_int n) -> CBTNode* {
        return CBTSequence::Create({
            CBTCondition::Create([n](CBlackboard* pBB) {
                return pBB->Get<_int>("ComboPick", 0) == n; }),
            Make_SwordCombo(n),
            });
        };

    return CBTSequence::Create({ pRoll,
        CBTSelector::Create({ Branch(1), Branch(2), Branch(3) }),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_GigaFly()
{
    auto bOn = make_shared<bool>(false);
    auto iPhase = make_shared<_int>(0);     // 0=상승, 1=수평 비행, 2=하강
    auto vGoal = make_shared<_float3>();
    auto vCorner = make_shared<_float3>();
    auto vDirLaunch = make_shared<_float3>();

    return CBTAction::Create(
        [this, bOn, iPhase, vGoal, vCorner, vDirLaunch](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);

            if (!*bOn)
            {
                _vector vK = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
                _float fBest = -1.f; _uint iBest = 0;
                for (_uint i = 0; i < CBoss_Metaknight::GIGA_POINT_COUNT; ++i)
                {
                    _vector vP = XMLoadFloat3(&CBoss_Metaknight::s_vGigaPoints[i]);
                    const _float d = XMVectorGetX(XMVector3LengthSq(XMVectorSetY(vP - vK, 0.f)));
                    if (d > fBest) { fBest = d; iBest = i; }
                }
                *vCorner = CBoss_Metaknight::s_vGigaPoints[iBest];

                Anim()->Play("Jump", false, true, 0.1f, SPD);
                mv->Set_GravityEnabled(false);
                XMStoreFloat3(vGoal.get(), vSelf + XMVectorSet(0.f, GIGA_FLY_H, 0.f, 0.f));
                *iPhase = 0;
                *bOn = true;
            }

            mv->Set_GravityEnabled(false);

            if (*iPhase == 2)
            {
                const _float fRemain = XMVectorGetX(XMVector3Length(
                    XMLoadFloat3(vGoal.get()) - vSelf));
                const _float t = SmoothStep01(1.f - fRemain / GIGA_FLY_H);

                _vector vDirK = Dir_ToTargetXZ();
                _vector vBlend = XMLoadFloat3(vDirLaunch.get()) * (1.f - t) + vDirK * t;
                if (XMVectorGetX(XMVector3LengthSq(vBlend)) > 1e-4f)
                    RotateYawTo(XMVector3Normalize(vBlend), 720.f, dt);
                else
                    RotateYawTo(vDirK, 720.f, dt);
            }

            const _float fSpeed = (*iPhase == 1) ? GIGA_FLY_SPEED : GIGA_RISE_SPEED;
            if (mv->Fly_Toward(XMLoadFloat3(vGoal.get()), fSpeed, dt, GIGA_ARRIVE))
            {
                if (*iPhase == 0)
                {
                    Anim()->Play("HoverDashStart", false, true, 0.15f, SPD);

                    CAnimator::ANI_PLAY_INFO tInfo{};
                    tInfo.strAniName = "HoverDash";
                    tInfo.bLoop = true;
                    tInfo.bRestart = true;
                    tInfo.fBlend = 0.1f;
                    tInfo.fSpeed = SPD;
                    Anim()->Enqueue(tInfo);

                    m_pOwner->Play_SectionLoopSFX(CBoss_Metaknight::SND_HOVERDASH, 0.13f, 0.6014f, 0.15f);
                    _vector vC = XMLoadFloat3(vCorner.get());
                    mv->Face_Instant(vC);
                    XMStoreFloat3(vGoal.get(), vC + XMVectorSet(0.f, GIGA_FLY_H, 0.f, 0.f));
                    *iPhase = 1;
                }
                else if (*iPhase == 1)
                {
                    Anim()->Play("HoverDashEnd", false, true, 0.15f, SPD);
                    m_pOwner->Release_LoopSFX(CBoss_Metaknight::SND_HOVERDASH);

                    _vector vLook = XMVector3Normalize(XMVectorSetY(
                        m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
                    XMStoreFloat3(vDirLaunch.get(), vLook);

                    XMStoreFloat3(vGoal.get(), XMLoadFloat3(vCorner.get()));
                    *iPhase = 2;
                }
                else
                {
                    mv->Set_GravityEnabled(true);
                    *bOn = false; *iPhase = 0;
                    return BT_STATUS::SUCCESS;
                }
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn, iPhase] {
            if (!*bOn)
                return;

            *bOn = false; *iPhase = 0;
            m_pOwner->Get_Movement()->Set_GravityEnabled(true);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_GigaMoonShot()
{
    auto* pBegin = CBTAction::Create(
        [this](CBlackboard*, _float) {
            static_cast<CBoss_Metaknight*>(m_pOwner)->Set_AttackBusy(true);
            return BT_STATUS::SUCCESS;
        },
        [this] { static_cast<CBoss_Metaknight*>(m_pOwner)->Set_AttackBusy(false); });

    auto* pFace = CBTAction::Create([this](CBlackboard*, _float) {
        m_pOwner->Get_Movement()->Face_Instant(
            XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos));
        return BT_STATUS::SUCCESS;
        });

    auto* pEnd = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Metaknight*>(m_pOwner)->Start_PatternCooldowns(CBoss_Metaknight::s_fGigaCooldown);
        return BT_STATUS::SUCCESS;
        });

    return CBTSequence::Create({
        pBegin,
        Make_GigaFly(),
        Clip("Landing", SPD, 0.2f),
        pFace,
        Clip("GigaMoonCharge", SPD, 0.2f),
        Clip("GigaMoonShot", SPD, 0.2f),
        pEnd,
        });
}

_bool CBoss_Metaknight_Brain::FlyNoClip(_fvector vGoal, _float fSpeed, _float dt, _float fArrive)
{
    auto* mv = m_pOwner->Get_Movement();
    auto* tf = m_pOwner->Get_Transform();
    _vector vSelf = tf->Get_State(STATE::POSITION);
    _vector vTo = vGoal - vSelf;
    _float  fDist = XMVectorGetX(XMVector3Length(vTo));
    if (fDist <= fArrive)
        return true;

    _float fStep = min(fSpeed * dt, fDist);
    tf->Set_State(STATE::POSITION, vSelf + XMVector3Normalize(vTo) * fStep);
    mv->Sync_To_Controller();
    return false;
}

void CBoss_Metaknight_Brain::RiseToward(_float fTargetY, _float dt)
{
    auto* tf = m_pOwner->Get_Transform();
    _vector p = tf->Get_State(STATE::POSITION);
    _float  y = XMVectorGetY(p);
    if (y < fTargetY)
        tf->Set_State(STATE::POSITION, XMVectorSetY(p, min(y + ROCK_RISE_SPEED * dt, fTargetY)));
}

CBTNode* CBoss_Metaknight_Brain::Make_RockFly()
{
    auto on = make_shared<bool>(false);
    auto phase = make_shared<_int>(0);        // 0=제자리상승, 1=목표비행, 2=도착회전
    auto vGoal = make_shared<_float3>();

    return CBTAction::Create(
        [this, on, phase, vGoal](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            auto* tf = m_pOwner->Get_Transform();
            _vector vSelf = tf->Get_State(STATE::POSITION);
            const _vector kDest = XMVectorSet(0.f, 16.f, 25.f, 0.f);

            if (!*on) {
                Anim()->Play("Jump", false, true, 0.1f, SPD);
                mv->Set_GravityEnabled(false);
                XMStoreFloat3(vGoal.get(), vSelf + XMVectorSet(0.f, GIGA_FLY_H, 0.f, 0.f));
                *phase = 0; *on = true;
                return BT_STATUS::RUNNING;
            }

            if (*phase == 0)
            {
                if (FlyNoClip(XMLoadFloat3(vGoal.get()), GIGA_FLY_SPEED, dt, GIGA_ARRIVE))
                {
                    Anim()->Play("HoverDashStart", false, true, 0.15f, SPD);
                    CAnimator::ANI_PLAY_INFO info{};
                    info.strAniName = "HoverDash"; info.bLoop = true; info.bRestart = true;
                    info.fBlend = 0.1f; info.fSpeed = SPD;
                    Anim()->Enqueue(info);
                    m_pOwner->Play_SectionLoopSFX(CBoss_Metaknight::SND_HOVERDASH, 0.13f, 0.6014f, 0.15f);
                    *phase = 1;
                }
            }
            else if (*phase == 1)
            {
                _vector vDir = XMVectorSetY(kDest - vSelf, 0.f);
                if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-4f)
                    RotateYawTo(XMVector3Normalize(vDir), 720.f, dt);

                if (FlyNoClip(kDest, GIGA_FLY_SPEED, dt, GIGA_ARRIVE))
                {
                    Anim()->Play("HoverDashEnd", false, true, 0.15f, SPD);
                    m_pOwner->Stop_LoopSFX(CBoss_Metaknight::SND_HOVERDASH);
                    *phase = 2;
                }
            }
            else
            {
                if (RotateYawTo(XMVectorSet(0.f, 0.f, -1.f, 0.f), 720.f, dt))
                {
                    *on = false; *phase = 0;
                    return BT_STATUS::SUCCESS;
                }
            }
            return BT_STATUS::RUNNING;
        },
        [this, on, phase] { *on = false; *phase = 0; });
}

CBTNode* CBoss_Metaknight_Brain::Make_RockDrop()
{
    auto risen = make_shared<_float>(0.f);

    auto* pBegin = CBTAction::Create(
        [this](CBlackboard*, _float) {
            static_cast<CBoss_Metaknight*>(m_pOwner)->Set_AttackBusy(true);
            return BT_STATUS::SUCCESS;
        },
        [this] { static_cast<CBoss_Metaknight*>(m_pOwner)->Set_AttackBusy(false); });

    auto startOn = make_shared<bool>(false);
    auto* pStartRise = CBTAction::Create(
        [this, startOn, risen](CBlackboard*, _float dt) -> BT_STATUS {
            if (!*startOn) {
                Anim()->Play("BurstTornadoStart", false, true, 0.2f, SPD);
                m_pOwner->Play_SectionLoopSFX(CBoss_Metaknight::SND_BURSTTORNADOATTACK, 0.1299f, 0.5198f, 0.2f);
                *risen = 0.f;                       // 누적 리셋
                *startOn = true;
                return BT_STATUS::RUNNING;
            }
            if (*risen < ROCK_RISE_HEIGHT) {
                _float step = min(ROCK_RISE_SPEED * dt, ROCK_RISE_HEIGHT - *risen);
                auto* tf = m_pOwner->Get_Transform();
                _vector p = tf->Get_State(STATE::POSITION);
                tf->Set_State(STATE::POSITION, p + XMVectorSet(0.f, step, 0.f, 0.f));
                m_pOwner->Get_Movement()->Sync_To_Controller();
                *risen += step;
            }
            if (Anim()->Is_Finished()) { *startOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [this, startOn] { *startOn = false; });

    auto waitOn = make_shared<bool>(false);
    auto waitTimer = make_shared<_float>(0.f);
    auto* pWaitRise = CBTAction::Create(
        [this, waitOn, waitTimer, risen](CBlackboard*, _float dt) -> BT_STATUS {
            if (!*waitOn) {
                Anim()->Play("BurstTornadoWait", true, true, 0.2f, SPD);
                *waitTimer = 0.f; *waitOn = true;
                return BT_STATUS::RUNNING;
            }
            if (*risen < ROCK_RISE_HEIGHT) {
                _float step = min(ROCK_RISE_SPEED * dt, ROCK_RISE_HEIGHT - *risen);
                auto* tf = m_pOwner->Get_Transform();
                _vector p = tf->Get_State(STATE::POSITION);
                tf->Set_State(STATE::POSITION, p + XMVectorSet(0.f, step, 0.f, 0.f));
                m_pOwner->Get_Movement()->Sync_To_Controller();
                *risen += step;
            }
            *waitTimer += dt;
            if (*waitTimer >= 2.f) { *waitOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [this, waitOn] { *waitOn = false; });

    auto atkOn = make_shared<bool>(false);
    auto* pAttack = CBTAction::Create(
        [this, atkOn](CBlackboard*, _float) -> BT_STATUS {
            if (!*atkOn) {
                auto meta = static_cast<CBoss_Metaknight*>(m_pOwner);
                Anim()->Play("BurstTornadoAttack", false, true, 0.2f, SPD);
                meta->Begin_RockDecalSlide();
                meta->Set_TopViewCam(true);
                m_pOwner->Stop_LoopSFX(CBoss_Metaknight::SND_BURSTTORNADOATTACK);
                *atkOn = true;
            }
            if (Anim()->Is_Finished()) { *atkOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [this, atkOn] { *atkOn = false; });

    auto tPre = make_shared<_float>(0.f);
    auto* pPreDrop = CBTAction::Create([tPre](CBlackboard*, _float dt) -> BT_STATUS {
        *tPre += dt;
        return (*tPre >= 3.f) ? BT_STATUS::SUCCESS : BT_STATUS::RUNNING;
        }, [tPre] { *tPre = 0.f; });

    auto* pDrop = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Metaknight*>(m_pOwner)->Drop_Rocks();
        return BT_STATUS::SUCCESS;
        });

    auto tLand = make_shared<_float>(0.f);
    auto* pWaitLanded = CBTAction::Create([tLand](CBlackboard*, _float dt) -> BT_STATUS {
        *tLand += dt;
        return (*tLand >= 2.5f) ? BT_STATUS::SUCCESS : BT_STATUS::RUNNING;
        }, [tLand] { *tLand = 0.f; });

    auto descOn = make_shared<bool>(false);
    auto* pDescendToTrack = CBTAction::Create(
        [this, descOn](CBlackboard*, _float dt) -> BT_STATUS {
            auto* tf = m_pOwner->Get_Transform();
            auto* mv = m_pOwner->Get_Movement();
            if (!*descOn) { 
                Anim()->Play("FlightWait", true, true, 0.2f, SPD);
                static_cast<CBoss_Metaknight*>(m_pOwner)->Set_TopViewCam(false);
                *descOn = true; 
            }

            _vector p = tf->Get_State(STATE::POSITION);
            _float  y = XMVectorGetY(p);
            if (y <= ROCK_TRACK_Y) { *descOn = false; return BT_STATUS::SUCCESS; }

            y = max(y - TRACK_DESCEND_SPEED * dt, ROCK_TRACK_Y);
            tf->Set_State(STATE::POSITION, XMVectorSetY(p, y));
            mv->Sync_To_Controller();
            return BT_STATUS::RUNNING;
        },
        [descOn] { *descOn = false; });

    auto trackOn = make_shared<bool>(false);
    auto* pFlyToKirby = CBTAction::Create(
        [this, trackOn](CBlackboard*, _float dt) -> BT_STATUS {
            auto* tf = m_pOwner->Get_Transform();
            if (!*trackOn) { Anim()->Play("FlightWait", true, true, 0.2f, SPD); *trackOn = true; }

            _vector vSelf = tf->Get_State(STATE::POSITION);
            _vector vKirby = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);

            _vector vDir = XMVectorSetY(vKirby - vSelf, 0.f);
            if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-4f)
                RotateYawTo(XMVector3Normalize(vDir), 360.f, dt);
            _vector vGoal = XMVectorSetY(vKirby, XMVectorGetY(vSelf));
            FlyNoClip(vGoal, ROCK_TRACK_SPEED, dt, 0.05f);

            _float fHoriz = XMVectorGetX(XMVector3Length(XMVectorSetY(vKirby - vSelf, 0.f)));
            if (fHoriz <= DIVEBOMB_RANGE) { *trackOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [trackOn] { *trackOn = false; });

    auto fallOn = make_shared<bool>(false);
    auto fallHold = make_shared<_float>(0.f);
    auto* pDiveFall = CBTAction::Create(
        [this, fallOn, fallHold](CBlackboard*, _float dt) -> BT_STATUS {
            auto* tf = m_pOwner->Get_Transform();
            auto* mv = m_pOwner->Get_Movement();

            if (!*fallOn) {
                Anim()->Play("DiveBombFall", true, true, 0.1f, SPD);
                *fallOn = true;
                *fallHold = 0.f;
                return BT_STATUS::RUNNING;
            }

            if (*fallHold < DIVEBOMB_FALL_HOLD) {
                *fallHold += dt;
                return BT_STATUS::RUNNING;
            }

            _vector p = tf->Get_State(STATE::POSITION);
            _float floorY = CBoss_Metaknight::s_vGigaPoints[0].y;
            _float y = XMVectorGetY(p) - DIVEBOMB_FALL_SPEED * dt;

            if (y <= floorY) {
                tf->Set_State(STATE::POSITION, XMVectorSetY(p, floorY));
                mv->Sync_To_Controller();
                *fallOn = false;
                *fallHold = 0.f;
                return BT_STATUS::SUCCESS;
            }
            tf->Set_State(STATE::POSITION, XMVectorSetY(p, y));
            mv->Sync_To_Controller();
            return BT_STATUS::RUNNING;
        },
        [fallOn, fallHold] { *fallOn = false; *fallHold = 0.f; });

    auto* pEnd = CBTAction::Create([this](CBlackboard*, _float) {
        m_pOwner->Get_Movement()->Set_GravityEnabled(true);
        auto meta = static_cast<CBoss_Metaknight*>(m_pOwner);
        meta->Set_TopViewCam(false);
        meta->Start_PatternCooldowns(CBoss_Metaknight::s_fRockCooldown);
        return BT_STATUS::SUCCESS;
        });

    return CBTSequence::Create({
        pBegin,
        Make_RockFly(),
        pStartRise,
        pWaitRise,
        pAttack,
        pPreDrop,
        pDrop,
        pWaitLanded,
        pDescendToTrack,             
        pFlyToKirby,                 
        Clip("DiveBombSomersault", SPD, 0.2f),
        pDiveFall,
        Clip("DiveBombEnd", SPD, 0.2f),
        pEnd,
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UpperCalibur()
{
    // 러시 노드와 분기 노드가 공유하는 상태
    auto bCaught = make_shared<bool>(false);
    auto vRushDir = make_shared<_float3>(_float3(0.f, 0.f, 1.f));

    auto* pBegin = CBTAction::Create(
        [this, bCaught](CBlackboard*, _float) {
            auto* pMeta = static_cast<CBoss_Metaknight*>(m_pOwner);
            pMeta->Set_AttackBusy(true);
            pMeta->Set_ParryWindow(true);
            pMeta->Reset_CatchHit();
            *bCaught = false;
            return BT_STATUS::SUCCESS;
        },
        [this] { 
            auto* pMeta = static_cast<CBoss_Metaknight*>(m_pOwner);
            pMeta->Set_AttackBusy(false);
            pMeta->Set_ParryWindow(false);
        });

    auto* pEnd = CBTAction::Create([this](CBlackboard*, _float) {
        auto* pMeta = static_cast<CBoss_Metaknight*>(m_pOwner);
        pMeta->Enable_CatchBox(false);
        pMeta->Set_AttackBusy(false);
        m_pOwner->Get_Movement()->Set_MoveSpeed(BASE_SPEED);
        m_pOwner->Get_Movement()->Set_LockFacing(false);
        pMeta->Start_PatternCooldowns(CBoss_Metaknight::s_fUpperCooldown);
        return BT_STATUS::SUCCESS;
        });

    auto Branch = [](shared_ptr<bool> b, _bool bWant, CBTNode* pBody) -> CBTNode* {
        return CBTSequence::Create({
            CBTCondition::Create([b, bWant](CBlackboard*) { return *b == bWant; }),
            pBody,
            });
        };

    return CBTSequence::Create({
        pBegin,
        Make_UC_BackStep(),
        Make_UC_Charge(),
        Make_UC_Rush(bCaught, vRushDir),
        CBTSelector::Create({
            Branch(bCaught, true,  Make_UC_CatchSuccess()),
            Branch(bCaught, false, Make_UC_Brake(vRushDir)),
        }),
        pEnd,
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UC_BackStep()
{
    auto bOn = make_shared<bool>(false);

    return CBTAction::Create(
        [this, bOn](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            if (!*bOn)
            {
                Anim()->Play("UpperCaliburRaisingStep", false, true, 0.15f, SPD);
                mv->Set_LockFacing(true);   // 뒤로 가면서도 커비를 계속 봐야 하므로 자동 페이싱 차단
                *bOn = true;
            }

            RotateYawTo(Dir_ToTargetXZ(), TURN_DEG, dt);

            // 클립 진행도에 맞춰 살짝 뒤로 (LOOK 반대 방향)
            mv->Set_WindowMoveSpeed(UC_BACK_SPEED, Anim()->Get_Progress());
            m_pOwner->Add_MoveDir(-m_pOwner->Get_Transform()->Get_State(STATE::LOOK));

            if (Anim()->Is_Finished())
            {
                mv->Set_LockFacing(false);
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn] {
            *bOn = false;
            m_pOwner->Get_Movement()->Set_LockFacing(false);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UC_Charge()
{
    auto bOn = make_shared<bool>(false);

    return CBTAction::Create(
        [this, bOn](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            if (!*bOn)
            {
                Anim()->Play("UpperCaliburRaisingCharge", false, true, 0.15f, SPD);
                mv->Set_LockFacing(true);
                *bOn = true;
            }

            RotateYawTo(Dir_ToTargetXZ(), TURN_DEG, dt);

            if (Anim()->Is_Finished())
            {
                mv->Set_LockFacing(false);
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn] {
            *bOn = false;
            m_pOwner->Get_Movement()->Set_LockFacing(false);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UC_Rush(shared_ptr<bool> bCaught, shared_ptr<_float3> vDir)
{
    auto bOn = make_shared<bool>(false);
    auto fT = make_shared<_float>(0.f);
    auto vStart = make_shared<_float3>();

    return CBTAction::Create(
        [this, bOn, fT, vStart, bCaught, vDir](CBlackboard*, _float dt) -> BT_STATUS {
            auto* pMeta = static_cast<CBoss_Metaknight*>(m_pOwner);
            auto* mv = m_pOwner->Get_Movement();
            auto* tf = m_pOwner->Get_Transform();

            if (!*bOn)
            {
                mv->Face_Instant(XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos));
                mv->Set_LockFacing(true);

                XMStoreFloat3(vDir.get(),
                    XMVector3Normalize(XMVectorSetY(tf->Get_State(STATE::LOOK), 0.f)));
                XMStoreFloat3(vStart.get(), tf->Get_State(STATE::POSITION));

                Anim()->Play("UpperCaliburRaisingMove", true, true, 0.1f, SPD);

                pMeta->Reset_CatchHit();
                pMeta->Enable_CatchBox(true);

                *bCaught = false;
                *fT = 0.f;
                *bOn = true;
            }

            *fT += dt;
            mv->Set_MoveSpeed(UC_RUSH_SPEED);
            m_pOwner->Add_MoveDir(XMLoadFloat3(vDir.get()));

            if (pMeta->Is_CatchHit())
            {
                pMeta->Enable_CatchBox(false);
                *bCaught = true;
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }

            const _float fRun = XMVectorGetX(XMVector3Length(
                XMVectorSetY(tf->Get_State(STATE::POSITION) - XMLoadFloat3(vStart.get()), 0.f)));

            if (fRun >= UC_RUSH_MAX_DIST || *fT >= UC_RUSH_TIMEOUT)
            {
                pMeta->Enable_CatchBox(false);
                *bCaught = false;
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn, fT] {
            *bOn = false; *fT = 0.f;
            auto* pMeta = static_cast<CBoss_Metaknight*>(m_pOwner);
            pMeta->Enable_CatchBox(false);
            m_pOwner->Get_Movement()->Set_MoveSpeed(BASE_SPEED);
            m_pOwner->Get_Movement()->Set_LockFacing(false);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UC_Brake(shared_ptr<_float3> vDir)
{
    auto bOn = make_shared<bool>(false);

    return CBTAction::Create(
        [this, bOn, vDir](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            if (!*bOn)
            {
                Anim()->Play("UpperCaliburBrake", false, true, 0.1f, SPD);
                mv->Set_LockFacing(true);
                *bOn = true;
            }

            const _float p = Anim()->Get_Progress();
            const _float fCur = UC_BRAKE_ENTRY_SPEED * powf(1.f - p, UC_BRAKE_DECAY_POW);

            mv->Set_MoveSpeed(fCur);
            m_pOwner->Add_MoveDir(XMLoadFloat3(vDir.get()));

            if (p >= 0.15f)
                RotateYawTo(XMVectorNegate(XMLoadFloat3(vDir.get())), UC_BRAKE_TURN_DEG, dt);

            if (Anim()->Is_Finished())
            {
                mv->Set_MoveSpeed(BASE_SPEED);
                mv->Set_LockFacing(false);
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn] {
            *bOn = false;
            m_pOwner->Get_Movement()->Set_MoveSpeed(BASE_SPEED);
            m_pOwner->Get_Movement()->Set_LockFacing(false);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UC_CatchSuccess()
{
    auto bOn = make_shared<bool>(false);

    auto* pDemo = CBTAction::Create(
        [this, bOn](CBlackboard*, _float) -> BT_STATUS {
            if (!*bOn)
            {
                auto* mv = m_pOwner->Get_Movement();
                mv->Set_MoveSpeed(BASE_SPEED);
                mv->Set_LockFacing(false);

                static_cast<CBoss_Metaknight*>(m_pOwner)->Begin_Demo(CBoss_Metaknight::EDemo::UPPER_CALIBUR);
                *bOn = true;
                return BT_STATUS::RUNNING;
            }

            if (Anim()->Is_Finished())
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bOn] { *bOn = false; });

    auto* pEnd = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Metaknight*>(m_pOwner)->End_Demo(CBoss_Metaknight::EDemo::UPPER_CALIBUR);
        return BT_STATUS::SUCCESS;
        });

    return CBTSequence::Create({
        pDemo,
        pEnd,
        Make_UC_Fall(),
        Clip("Landing", SPD, 0.1f),
        Clip("Wait", SPD, 0.2f),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UC_Fall()
{
    auto bOn = make_shared<bool>(false);
    auto fT = make_shared<_float>(0.f);

    return CBTAction::Create(
        [this, bOn, fT](CBlackboard*, _float dt) -> BT_STATUS {
            if (!*bOn)
            {
                Anim()->Play("Fall", true, true, 0.f, SPD);
                *fT = 0.f;
                *bOn = true;
                return BT_STATUS::RUNNING;
            }

            *fT += dt;

            if (*fT < UC_FALL_MIN_TIME)
                return BT_STATUS::RUNNING;

            if (m_pOwner->Get_Movement()->Is_Grounded() || *fT >= UC_FALL_TIMEOUT)
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bOn, fT] { *bOn = false; *fT = 0.f; });
}

CBTNode* CBoss_Metaknight_Brain::Make_LockLose()
{
    auto bOn = make_shared<bool>(false);

    auto* pDemo = CBTAction::Create(
        [this, bOn](CBlackboard*, _float) -> BT_STATUS {
            if (!*bOn)
            {
                static_cast<CBoss_Metaknight*>(m_pOwner)->Begin_Demo(CBoss_Metaknight::EDemo::LOCK_LOSE);
                *bOn = true;
                return BT_STATUS::RUNNING;
            }
            if (Anim()->Is_Finished()) { *bOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [bOn] { *bOn = false; });

    auto* pEnd = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Metaknight*>(m_pOwner)->End_Demo(CBoss_Metaknight::EDemo::LOCK_LOSE);
        return BT_STATUS::SUCCESS;
        });

    return CBTSequence::Create({
        pDemo,
        pEnd,
        Loop("LockingSwordLoseWait", 5.f, SPD),
        Make_LockLose_Return(),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_LockLose_Return()
{
    auto* pHoldOn = CBTAction::Create(
        [this](CBlackboard*, _float) {
            static_cast<CBoss_Metaknight*>(m_pOwner)->Hold_BossCam(true);
            return BT_STATUS::SUCCESS;
        },
        [this] { static_cast<CBoss_Metaknight*>(m_pOwner)->Hold_BossCam(false); });

    auto* pHoldOff = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Metaknight*>(m_pOwner)->Hold_BossCam(false);
        return BT_STATUS::SUCCESS;
        });

    auto* pReplicaOn = CBTAction::Create([this](CBlackboard*, _float) {
        auto* mk = static_cast<CBoss_Metaknight*>(m_pOwner);
        mk->Retire_Galaxia();
        mk->Set_ActiveSword(CBoss_Metaknight::EMK_SWORD::REPLICA);
        return BT_STATUS::SUCCESS;
        });

    auto* pFallBegin = CBTAction::Create([this](CBlackboard*, _float) {
        m_pOwner->Get_Movement()->Set_GravityEnabled(true);
        return BT_STATUS::SUCCESS;
        });

    return CBTSequence::Create({
        pHoldOn,                        // 보스캠 고정 시작
        Clip("JumpStart", SPD, 0.1f),   // 점프스타트(윈드업)
        Make_ReturnFly(),               // 점프 상승 + 맵중앙 수평이동
        pReplicaOn,                     // 레플리카 소드 on + 바닥 갤럭시아 정리
        pFallBegin,                     // 중력 on
        Make_UC_Fall(),                 // Fall + Is_Grounded 대기(재사용)
        Clip("Landing", SPD, 0.1f),     // 착지
        pHoldOff,                       // 보스캠 추적 재개
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_ReturnFly()
{
    auto bOn = make_shared<bool>(false);
    auto iPhase = make_shared<_int>(0);   // 0=상승, 1=맵중앙 수평이동
    auto vGoal = make_shared<_float3>();

    return CBTAction::Create(
        [this, bOn, iPhase, vGoal](CBlackboard*, _float dt) -> BT_STATUS {
            auto* mv = m_pOwner->Get_Movement();
            _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);

            if (!*bOn)
            {
                Anim()->Play("Jump", false, true, 0.1f, SPD);
                mv->Set_GravityEnabled(false);
                XMStoreFloat3(vGoal.get(), vSelf + XMVectorSet(0.f, RET_RISE_H, 0.f, 0.f));
                *iPhase = 0;
                *bOn = true;
            }

            const _float fSpeed = (*iPhase == 0) ? RET_RISE_SPEED : RET_MOVE_SPEED;
            if (mv->Fly_Toward(XMLoadFloat3(vGoal.get()), fSpeed, dt, GIGA_ARRIVE))
            {
                if (*iPhase == 0)
                {
                    _float fY = XMVectorGetY(m_pOwner->Get_Transform()->Get_State(STATE::POSITION));
                    _vector vC = XMVectorSet(0.f, fY, 0.f, 1.f);
                    mv->Face_Instant(vC);
                    XMStoreFloat3(vGoal.get(), vC);
                    *iPhase = 1;
                }
                else
                {
                    *bOn = false; *iPhase = 0;
                    return BT_STATUS::SUCCESS;
                }
            }
            return BT_STATUS::RUNNING;
        },
        [this, bOn, iPhase] {
            if (!*bOn)
                return;

            *bOn = false; *iPhase = 0;
            m_pOwner->Get_Movement()->Set_GravityEnabled(true);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_DodgeBranch()
{
    return CBTSequence::Create({
        CBTCondition::Create([this](CBlackboard*) {
            return static_cast<CBoss_Metaknight*>(m_pOwner)->Consume_DodgeRequest(); }),
        Make_Dodge(),
        Clip("Wait", SPD, 0.2f),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_GigaBranch()
{
    return Make_Optional(
        CBTCondition::Create([this](CBlackboard* pBB) {
            return static_cast<CBoss_Metaknight*>(m_pOwner)->Is_GigaReady()
                && pBB->Get<_float>("DistToTarget", FLT_MAX) > COMBO_RANGE; }),
        Make_GigaMoonShot());
}

CBTNode* CBoss_Metaknight_Brain::Make_ComboBranch()
{
    return Make_Optional(
        CBTCondition::Create([](CBlackboard* pBB) {
            return pBB->Get<_float>("DistToTarget", FLT_MAX) <= COMBO_RANGE; }),
            Make_ComboPick());
}

CBTNode* CBoss_Metaknight_Brain::Make_UpperBranch()
{
    return CBTSequence::Create({
        CBTCondition::Create([this](CBlackboard* pBB) {
            return static_cast<CBoss_Metaknight*>(m_pOwner)->Is_UpperReady()
                && pBB->Get<_float>("DistToTarget", FLT_MAX) > COMBO_RANGE; }),
        Make_UpperCalibur(),
        Clip("Wait", SPD, 0.2f),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_LockOutcomeBranch()
{
    auto iOut = make_shared<_int>(0);

    auto* pRoll = CBTAction::Create([this, iOut](CBlackboard*, _float) {
        auto e = static_cast<CBoss_Metaknight*>(m_pOwner)->Consume_LockOutcome();
        *iOut = (e == CBoss_Metaknight::ELockOutcome::MK_WIN) ? 1
            : (e == CBoss_Metaknight::ELockOutcome::MK_LOSE) ? 2 : 0;
        return (*iOut != 0) ? BT_STATUS::SUCCESS : BT_STATUS::FAILURE;
        });

    auto Branch = [](shared_ptr<_int> r, _int want, CBTNode* pBody) -> CBTNode* {
        return CBTSequence::Create({
            CBTCondition::Create([r, want](CBlackboard*) { return *r == want; }),
            pBody,
            });
        };

    return CBTSequence::Create({
        pRoll,
        CBTSelector::Create({
            Branch(iOut, 1, Make_UC_CatchSuccess()),
            Branch(iOut, 2, Make_LockLose()),       
        }),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_RockBranch()
{
    return CBTSequence::Create({
        CBTCondition::Create([this](CBlackboard*) {
            return static_cast<CBoss_Metaknight*>(m_pOwner)->Is_RockReady(); }),
        Make_RockDrop(),
        Clip("Wait", SPD, 0.2f),
        });
}

CBoss_Metaknight_Brain* CBoss_Metaknight_Brain::Create(CMonster* pOwner)
{
    CBoss_Metaknight_Brain* pInstance = new CBoss_Metaknight_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner))) {
        MSG_BOX("Failed to Created: CBoss_Metaknight_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight_Brain::Free() { __super::Free(); }