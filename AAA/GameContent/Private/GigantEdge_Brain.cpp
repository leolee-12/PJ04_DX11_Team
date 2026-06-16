#include "GigantEdge_Brain.h"
#include "GigantEdge.h"
#include "GigantEdge_Body.h"

#include "GameInstance.h"
#include "Animator.h"
#include "Transform.h"

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
    constexpr _float fGuardTime = 2.f;

    // 1회성
    auto MakeOneShot = [this](const string& strClip) -> CBTAction*
        {
            auto bStarted = make_shared<bool>(false);
            return CBTAction::Create(
                [this, strClip, bStarted](CBlackboard*, _float) -> BT_STATUS
                {
                    auto* a = m_pOwner->Get_Body()->Get_Animator();
                    if (!*bStarted) { a->Play(strClip, false, true); *bStarted = true; }
                    if (a->Is_Finished()) { *bStarted = false; return BT_STATUS::SUCCESS; }
                    return BT_STATUS::RUNNING;
                },
                [bStarted]() { *bStarted = false; });
        };

    // 그로기
    auto bGroggy = make_shared<bool>(false);
    auto* pGroggy = CBTAction::Create(
        [this, bGroggy](CBlackboard*, _float) -> BT_STATUS
        {
            auto* a = m_pOwner->Get_Body()->Get_Animator();
            if (!*bGroggy) { a->Play("Groggy", false, true); *bGroggy = true; }
            if (a->Is_Finished()) { *bGroggy = false; m_pOwner->Clear_Groggy(); return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [bGroggy]() { *bGroggy = false; });

    // 추격 
    auto bWalk = make_shared<bool>(false);
    auto* pChase = CBTAction::Create(
        [this, bWalk](CBlackboard* pBB, _float) -> BT_STATUS
        {
            if (!*bWalk) { m_pOwner->Get_Body()->Get_Animator()->Play("Walk", true, true); *bWalk = true; }
            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            m_pOwner->Add_MoveDir(vDir);                 // CMonster가 Move 적용
            return BT_STATUS::RUNNING;
        },
        [bWalk]() { *bWalk = false; });

    // 공격 선택 3가지
    auto* pPick = CBTAction::Create([this](CBlackboard* pBB, _float)
        { pBB->Set<_int>("AttackChoice", (_int)(rand() % 3)); return BT_STATUS::SUCCESS; });
    auto MakeAttack = [&](_int i, const string& clip) -> CBTNode*
        {
            return CBTSequence::Create({
                CBTCondition::Create([i](CBlackboard* pBB) { return pBB->Get<_int>("AttackChoice", -1) == i; }),
                MakeOneShot(clip),
                });
        };

    // 가드
    auto fGuardT = make_shared<_float>(0.f);
    auto* pGuard = CBTAction::Create(
        [this, fGuardT, fGuardTime](CBlackboard*, _float fDt) -> BT_STATUS
        {
            if (*fGuardT == 0.f) { m_pOwner->Get_Body()->Get_Animator()->Play("Guard", true, true); m_pOwner->Set_Guarding(true); }
            *fGuardT += fDt;
            if (*fGuardT >= fGuardTime) { *fGuardT = 0.f; m_pOwner->Set_Guarding(false); return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        },
        [this, fGuardT]() { *fGuardT = 0.f; m_pOwner->Set_Guarding(false); });   // 중단 시 가드 해제

    auto* pInRange = CBTCondition::Create([fAttackRange](CBlackboard* pBB)
        { return pBB->Get<_float>("DistToTarget", FLT_MAX) <= fAttackRange; });

    // 전투: 사거리안 -> 추격 끊고 공격
    CBTNode* pCombat = CBTReactiveSelector::Create({
        CBTSequence::Create({
            pInRange, pPick,
            CBTSelector::Create({ MakeAttack(0, "Attack_Slam"), MakeAttack(1, "Attack_Charge"), MakeAttack(2, "Attack_Swing") }),
            pGuard,
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