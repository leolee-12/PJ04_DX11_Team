#include "Boss_Gorilla_Brain.h"
#include "Boss.h"
#include "BT.h"

CBTNode* CBoss_Gorilla_Brain::Build_PhaseTree(_int iPhase)
{
    // TODO: 페이즈별 실제 패턴 (GigantEdge_Brain의 람다 빌더 패턴 재사용)
    //       지금은 모든 페이즈 동일 = 타깃 추적만.
    return CBTAction::Create(
        [this](CBlackboard* pBB, _float) -> BT_STATUS
        {
            m_pOwner->Add_MoveDir(pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f)));
            return BT_STATUS::RUNNING;
        });
}

CBoss_Gorilla_Brain* CBoss_Gorilla_Brain::Create()
{
    CBoss_Gorilla_Brain* pInstance = new CBoss_Gorilla_Brain();
    if (FAILED(pInstance->Initialize_Trees()))      // CBoss_Brain: 페이즈별 BT 구성
    {
        MSG_BOX("Failed to Created : CBoss_Gorilla_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}