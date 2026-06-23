#include "Boss_Brain.h"
#include "Boss.h"
#include "GameInstance.h"
#include "Transform.h"
#include "BT.h"

HRESULT CBoss_Brain::Initialize_Trees()
{
    for (_int i = 0; i < Get_PhaseCount(); ++i)
    {
        CBTNode* pRoot = Build_PhaseTree(i);
        if (nullptr == pRoot)
            return E_FAIL;

        CBehaviorTree* pBT = CBehaviorTree::Create(pRoot);
        if (nullptr == pBT)
            return E_FAIL;

        m_PhaseBTs.push_back(pBT);
    }
    return m_PhaseBTs.empty() ? E_FAIL : S_OK;
}

void CBoss_Brain::Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BB, _float fDt)
{
    m_pOwner = static_cast<CBoss*>(pMonster);

    _int iPhase = m_pOwner->Get_Phase();
    if (iPhase < 0 || iPhase >= static_cast<_int>(m_PhaseBTs.size()))
        return;

    CBehaviorTree* pBT = m_PhaseBTs[iPhase];
    auto* pBB = pBT->Get_Blackboard();

    pBB->Set<_float>("DistToTarget", BB.fDistToTarget);

    _vector vMyPos = pMonster->Get_Transform()->Get_State(STATE::POSITION);
    _vector vTgt = XMLoadFloat3(&BB.vTargetPos);
    _vector vDiff = XMVectorSetY(vTgt - vMyPos, 0.f);
    _float3 vDir;  XMStoreFloat3(&vDir, XMVector3Normalize(vDiff));
    pBB->Set<_float3>("DirToTarget", vDir);

    pBT->Tick(fDt);
}

void CBoss_Brain::Free()
{
    for (auto* pBT : m_PhaseBTs)
        Safe_Release(pBT);
    m_PhaseBTs.clear();

    __super::Free();
}