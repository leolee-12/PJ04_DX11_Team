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

CBTNode* CBoss_Metaknight_Brain::Build_PhaseTree(_int)
{
    return CBTReactiveSelector::Create({
        Make_DodgeBranch(),
        CBTSequence::Create({
            Make_RockBranch(),
            Make_GigaBranch(),
            Make_StepApproach(),
            Make_UnlessInRange(Make_DashIn()),
            Make_ComboBranch(),
        }),
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

CBTNode* CBoss_Metaknight_Brain::Make_DodgeBranch()
{
    return CBTSequence::Create({
        CBTCondition::Create([this](CBlackboard*) {
            return static_cast<CBoss_Metaknight*>(m_pOwner)->Consume_DodgeRequest(); }),
        Make_Dodge(),
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

                    _vector vC = XMLoadFloat3(vCorner.get());
                    mv->Face_Instant(vC);
                    XMStoreFloat3(vGoal.get(), vC + XMVectorSet(0.f, GIGA_FLY_H, 0.f, 0.f));
                    *iPhase = 1;
                }
                else if (*iPhase == 1)
                {
                    Anim()->Play("HoverDashEnd", false, true, 0.15f, SPD);

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
            *bOn = false; *iPhase = 0;
            m_pOwner->Get_Movement()->Set_GravityEnabled(true);
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_GigaMoonShot()
{
    auto* pBegin = CBTAction::Create(
        [this](CBlackboard*, _float) {
            auto* pBoss = static_cast<CBoss_Metaknight*>(m_pOwner);
            pBoss->Set_AttackBusy(true);
            pBoss->Start_GigaCooldown();
            return BT_STATUS::SUCCESS;
        },
        [this] { static_cast<CBoss_Metaknight*>(m_pOwner)->Set_AttackBusy(false); });

    auto* pFace = CBTAction::Create([this](CBlackboard*, _float) {
        m_pOwner->Get_Movement()->Face_Instant(
            XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos));
        return BT_STATUS::SUCCESS;
        });

    auto* pFire = CBTAction::Create([this](CBlackboard*, _float) {
        static_cast<CBoss_Metaknight*>(m_pOwner)->Fire_GigaMoonShot();
        return BT_STATUS::SUCCESS;
        });

    return CBTSequence::Create({
        pBegin,
        Make_GigaFly(),
        Clip("Landing", SPD, 0.2f),
        pFace,
        Clip("GigaMoonCharge", SPD, 0.2f),
        pFire,
        Clip("GigaMoonShot", SPD, 0.2f),
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

    auto startOn = make_shared<bool>(false);
    auto* pStartRise = CBTAction::Create(
        [this, startOn, risen](CBlackboard*, _float dt) -> BT_STATUS {
            if (!*startOn) {
                Anim()->Play("BurstTornadoStart", false, true, 0.2f, SPD);
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
            if (*waitTimer >= 3.f) { *waitOn = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [this, waitOn] { *waitOn = false; });

    auto atkOn = make_shared<bool>(false);
    auto* pAttack = CBTAction::Create(
        [this, atkOn](CBlackboard*, _float) -> BT_STATUS {
            if (!*atkOn) {
                Anim()->Play("BurstTornadoAttack", false, true, 0.2f, SPD);
                static_cast<CBoss_Metaknight*>(m_pOwner)->Begin_RockDecalSlide();
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
            if (!*descOn) { Anim()->Play("FlightWait", true, true, 0.2f, SPD); *descOn = true; }

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
    auto* pDiveFall = CBTAction::Create(
        [this, fallOn](CBlackboard*, _float dt) -> BT_STATUS {
            auto* tf = m_pOwner->Get_Transform();
            auto* mv = m_pOwner->Get_Movement();
            if (!*fallOn) {
                Anim()->Play("DiveBombFall", false, true, 0.1f, SPD);
                *fallOn = true;
                return BT_STATUS::RUNNING;
            }
            _vector p = tf->Get_State(STATE::POSITION);
            _float floorY = m_pOwner->Get_BlackBoard().vTargetPos.y;
            _float y = XMVectorGetY(p) - DIVEBOMB_FALL_SPEED * dt;

            if (y <= floorY) {
                tf->Set_State(STATE::POSITION, XMVectorSetY(p, floorY));
                mv->Sync_To_Controller();
                *fallOn = false;
                return BT_STATUS::SUCCESS;
            }
            tf->Set_State(STATE::POSITION, XMVectorSetY(p, y));
            mv->Sync_To_Controller();
            return BT_STATUS::RUNNING;
        },
        [fallOn] { *fallOn = false; });

    auto* pEnd = CBTAction::Create([this](CBlackboard*, _float) {
        m_pOwner->Get_Movement()->Set_GravityEnabled(true);
        return BT_STATUS::SUCCESS;
        });

    return CBTSequence::Create({
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

CBTNode* CBoss_Metaknight_Brain::Make_RockBranch()
{
    return CBTSequence::Create({
        CBTCondition::Create([this](CBlackboard*) {
            return static_cast<CBoss_Metaknight*>(m_pOwner)->Consume_RockDropRequest(); }),
        Make_RockDrop(),
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