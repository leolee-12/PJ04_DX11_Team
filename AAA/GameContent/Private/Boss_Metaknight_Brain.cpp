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
    CBTNode* pDodgeBranch = CBTSequence::Create({
        CBTCondition::Create([this](CBlackboard*) {
            return static_cast<CBoss_Metaknight*>(m_pOwner)->Consume_DodgeRequest(); }),
        Make_Dodge(),
        });

    return CBTReactiveSelector::Create({
        pDodgeBranch,
        CBTSequence::Create({
            Make_StepApproach(),
            Make_UnlessInRange(Make_DashIn()),
            CBTSelector::Create({
                CBTSequence::Create({
                    CBTCondition::Create([](CBlackboard* pBB) {
                        return pBB->Get<_float>("DistToTarget", FLT_MAX) <= COMBO_RANGE; }),
                    Make_ComboPick(),
                }),
                CBTCondition::Create([](CBlackboard*) { return true; }),
            }),
        }),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_UnlessInRange(CBTNode* pNode)
{
    return CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([](CBlackboard* pBB) {
                return pBB->Get<_float>("DistToTarget", FLT_MAX) > COMBO_RANGE; }),
            pNode,
        }),
        CBTCondition::Create([](CBlackboard*) { return true; }),
        });
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
        return CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([n](CBlackboard* pBB) {
                    return pBB->Get<_int>("StepCount", 1) >= n; }),
                Loop("Wait", STEP_PAUSE, SPD),
                Make_RandStep(),
            }),
            CBTCondition::Create([](CBlackboard*) { return true; }),
            });
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