#include "GigantEdge_Brain.h"
#include "GigantEdge.h"
#include "GigantEdge_Body.h"
#include "GameInstance.h"
#include "Animator.h"
#include "Transform.h"
#include "Monster_Movement.h"

#include "BT.h"

#include <memory>

void CGigantEdge_Brain::Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BB, _float fDt)
{
    m_pOwner = static_cast<CGigantEdge*>(pMonster);
    if (nullptr == m_pBT) return;

    auto* pBB = m_pBT->Get_Blackboard();
    pBB->Set<_float>("DistToTarget", BB.fDistToTarget);

    // 타깃 방향(XZ 평면)
    _vector vMyPos = pMonster->Get_Transform()->Get_State(STATE::POSITION);
    _vector vTgt = XMLoadFloat3(&BB.vTargetPos);
    _vector vDiff = XMVectorSetY(vTgt - vMyPos, 0.f);
    _float3 vDir;  XMStoreFloat3(&vDir, XMVector3Normalize(vDiff));
    pBB->Set<_float3>("DirToTarget", vDir);

    m_pBT->Tick(fDt);
}

HRESULT CGigantEdge_Brain::Initialize()
{
    constexpr _float fAttackRange = 4.f;
    constexpr _float fFacingDot = 0.86f;
    constexpr _float fTurnSpeedDeg = 180.f;
    constexpr _float fGuardTime = 4.f;
    constexpr _float fChargeTime = 0.8f;  
    constexpr _float fThrustCharge = 1.0f; 
    constexpr _int   iAttackCount = 3;

    auto OneShot = [this](const string& clip, _float fSpeed = 1.f) -> CBTNode*
        {
            auto started = make_shared<bool>(false);
            return CBTAction::Create(
                [this, clip, fSpeed, started](CBlackboard*, _float) -> BT_STATUS {
                    auto* a = m_pOwner->Get_Body()->Get_Animator();
                    if (!*started) { a->Play(clip, false, true, 0.2f, fSpeed); *started = true; }
                    if (a->Is_Finished()) { *started = false; return BT_STATUS::SUCCESS; }
                    return BT_STATUS::RUNNING;
                },
                [started]() { *started = false; });
        };

    auto HoldLoop = [this](const string& clip, _float fHold, _float fSpeed = 1.f) -> CBTNode*
        {
            auto t = make_shared<_float>(0.f);
            return CBTAction::Create(
                [this, clip, fHold, fSpeed, t](CBlackboard*, _float dt) -> BT_STATUS {
                    auto* a = m_pOwner->Get_Body()->Get_Animator();
                    if (*t == 0.f) a->Play(clip, true, true, 0.2f, fSpeed);
                    *t += dt;
                    if (*t >= fHold) { *t = 0.f; return BT_STATUS::SUCCESS; }
                    return BT_STATUS::RUNNING;
                },
                [t]() { *t = 0.f; });
        };

    auto MakeChargeAttack = [&]() -> CBTNode*     
        {
            return CBTSequence::Create({
                OneShot("SwordAppearStart", 2.f),
                OneShot("SwordAppear", 2.f),
                //OneShot("AttackAChargeStart", 2.f),
                HoldLoop("AttackAChargeWait", fChargeTime),
                OneShot("AttackAStart", 2.f),
                OneShot("AttackAWait", 2.f),
                OneShot("AttackAEnd", 2.f),
                });
        };

    auto MakeSideAttack = [&]() -> CBTNode*
        {
            return CBTSequence::Create({
                OneShot("SwordAppearStart", 2.f),
                OneShot("SideSwordAppear", 2.f),
                //OneShot("SideAttackAChargeStart", 2.f),
                HoldLoop("SideAttackAChargeWait", fChargeTime),
                OneShot("SideAttackAStart", 2.f),
                OneShot("SideAttackAWait", 2.f),
                OneShot("SideAttackAEnd", 2.f),
                });
        };

    auto MakeThrust = [&]() -> CBTNode* {
        auto bLunge = make_shared<bool>(false);
        auto fBaseSpd = make_shared<_float>(0.f);
        auto* pLunge = CBTAction::Create(
            [this, bLunge, fBaseSpd](CBlackboard*, _float) -> BT_STATUS {
                auto* a = m_pOwner->Get_Body()->Get_Animator();
                auto* mv = m_pOwner->Get_Movement();
                if (!*bLunge) {
                    a->Play("ThrustMove", false, true, 0.1f, 2.f);
                    *fBaseSpd = mv->Get_MoveSpeed();
                    mv->Set_MoveSpeed(12.f);          // ← 돌진 속도 (튜닝)
                    *bLunge = true;
                }
                _vector vLook = XMVector3Normalize(XMVectorSetY(
                    m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
                m_pOwner->Add_MoveDir(vLook);          // 정면 전진
                if (a->Is_Finished()) {
                    mv->Set_MoveSpeed(*fBaseSpd);      // 속도 복구
                    *bLunge = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [this, bLunge, fBaseSpd]() {                // 중단 시에도 복구
                if (*bLunge) { m_pOwner->Get_Movement()->Set_MoveSpeed(*fBaseSpd); *bLunge = false; }
            });

        return CBTSequence::Create({
            OneShot("SwordAppearStart", 2.f),
            OneShot("PreThrustStart", 2.f),
            HoldLoop("PreThrustStartWait", fThrustCharge),
            OneShot("ThrustStart", 2.f),
            pLunge,                          // ← 기존 OneShot("ThrustMove") 대체
            OneShot("ThrustEnd", 2.f),
            OneShot("ThrustEndWait", 2.f),
            OneShot("ThrustEndWaitEnd", 2.f),
            });
        };

    auto bGroggy = make_shared<bool>(false);
    auto* pGroggy = CBTAction::Create(
        [this, bGroggy](CBlackboard*, _float) -> BT_STATUS {
            auto* a = m_pOwner->Get_Body()->Get_Animator();
            if (!*bGroggy) { a->Play("ShieldGuardDamage", false, true, 0.2f, 2.f); *bGroggy = true; }
            if (a->Is_Finished()) { *bGroggy = false; m_pOwner->Clear_Groggy(); return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [bGroggy]() { *bGroggy = false; });

    auto bMove = make_shared<bool>(false);
    auto* pChase = CBTAction::Create(
        [this, bMove](CBlackboard* pBB, _float) -> BT_STATUS {
            if (!*bMove) { m_pOwner->Get_Body()->Get_Animator()->Play("Move", true, true, 0.2f, 2.f); *bMove = true; }
#ifdef _DEBUG
            if (!m_pOwner->Dbg_WalkInPlace())
#endif
                m_pOwner->Add_MoveDir(pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f)));
            return BT_STATUS::RUNNING;
        },
        [bMove]() { *bMove = false; });

    auto* pPick = CBTAction::Create([this, iAttackCount](CBlackboard* pBB, _float) {
        _int iChoice = (_int)(rand() % iAttackCount);
#ifdef _DEBUG
        if (m_pOwner->Dbg_ForceAttack() >= 0) iChoice = m_pOwner->Dbg_ForceAttack() % iAttackCount;
#endif
        pBB->Set<_int>("AttackChoice", iChoice);
        return BT_STATUS::SUCCESS;
        });
    auto MakeAttackBranch = [&](_int i, CBTNode* pAttack) -> CBTNode* {
        return CBTSequence::Create({
            CBTCondition::Create([i](CBlackboard* pBB) { return pBB->Get<_int>("AttackChoice", -1) == i; }),
            pAttack,
            });
        };

    auto fGuardT = make_shared<_float>(0.f);
    auto* pGuardHold = CBTAction::Create(
        [this, fGuardT, fGuardTime](CBlackboard*, _float dt) -> BT_STATUS {
            auto* a = m_pOwner->Get_Body()->Get_Animator();
            if (*fGuardT == 0.f) { a->Play("ShieldGuardWait", true, true); m_pOwner->Set_Guarding(true); }
            *fGuardT += dt;
            if (*fGuardT >= fGuardTime) { *fGuardT = 0.f; m_pOwner->Set_Guarding(false); return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [this, fGuardT]() { *fGuardT = 0.f; m_pOwner->Set_Guarding(false); });
    CBTNode* pGuard = CBTSequence::Create({
        OneShot("ShieldAppear", 2.f),
        OneShot("ShieldGuardStart"),
        pGuardHold,
        OneShot("ShieldHide"),
        });

    auto* pInRangeDist = CBTCondition::Create([this, fAttackRange](CBlackboard* pBB) {
#ifdef _DEBUG
        if (m_pOwner->Dbg_ForceInRange()) return true;
#endif
        return pBB->Get<_float>("DistToTarget", FLT_MAX) <= fAttackRange;
        });

    auto* pFacing = CBTCondition::Create([this, fFacingDot](CBlackboard* pBB) {
#ifdef _DEBUG
        if (m_pOwner->Dbg_ForceInRange()) return true;
#endif
        _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
        _vector vToTgt = XMLoadFloat3(&vDir);
        _vector vLook = XMVector3Normalize(XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
        return XMVectorGetX(XMVector3Dot(vLook, vToTgt)) >= fFacingDot;
        });

    auto bTurn = make_shared<bool>(false);
    auto* pTurnToTarget = CBTAction::Create(
        [this, bTurn, fTurnSpeedDeg, fFacingDot](CBlackboard* pBB, _float fDt) -> BT_STATUS {
            if (!*bTurn) { m_pOwner->Get_Body()->Get_Animator()->Play("Wait", true, true); *bTurn = true; }

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

    const _float fPostAtkDelay = 1.5f;
    auto fPostAtkT = make_shared<_float>(0.f);
    auto* pPostAttackDelay = CBTAction::Create(
        [this, fPostAtkT, fPostAtkDelay, fTurnSpeedDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (*fPostAtkT == 0.f)
                m_pOwner->Get_Body()->Get_Animator()->Play("Wait", true, true);
            *fPostAtkT += dt;

            // 대기 동안 플레이어 쪽으로 회전 (pTurnToTarget과 동일 로직)
            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vToTgt = XMLoadFloat3(&vDir);
            if (!XMVector3Equal(vToTgt, XMVectorZero()))
            {
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

            if (*fPostAtkT >= fPostAtkDelay) { *fPostAtkT = 0.f; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [fPostAtkT]() { *fPostAtkT = 0.f; });

    CBTNode* pCombat = CBTReactiveSelector::Create({
        CBTSequence::Create({
            pInRangeDist,                              
            CBTReactiveSelector::Create({
                CBTSequence::Create({                
                    pFacing, pPick,
                    CBTSelector::Create({
                        MakeAttackBranch(0, MakeChargeAttack()),       
                        MakeAttackBranch(1, MakeSideAttack()),   
                        MakeAttackBranch(2, MakeThrust()),                      
                    }),
                    pPostAttackDelay,
                    pGuard,
                }),
                pTurnToTarget,                         
            }),
        }),
        pChase,                                        
    });

    CBTNode* pRoot = CBTReactiveSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard*) { return m_pOwner->Is_GroggyRequested(); }),
            pGroggy,
        }),
        pCombat,
        });

    m_pBT = CBehaviorTree::Create(pRoot);
    return m_pBT ? S_OK : E_FAIL;
}

CGigantEdge_Brain* CGigantEdge_Brain::Create()
{
    CGigantEdge_Brain* pInstance = new CGigantEdge_Brain();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CGigantEdge_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGigantEdge_Brain::Free()
{
    Safe_Release(m_pBT);
    __super::Free();
}