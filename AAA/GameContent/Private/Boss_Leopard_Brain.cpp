#include "Boss_Leopard_Brain.h"
#include "BT.h"

CBTNode* CBoss_Leopard_Brain::Build_PhaseTree(_int iPhase)
{
    UNREFERENCED_PARAMETER(iPhase);
    // 깡통 브레인: 아무 행동 없이 계속 대기 (전투 패턴은 다음 단계)
    return CBTAction::Create(
        [](CBlackboard*, _float) -> BT_STATUS { return BT_STATUS::RUNNING; });
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