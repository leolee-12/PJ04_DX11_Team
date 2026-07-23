#include "BehaviorTree.h"

#include "BTNode.h"
#include "Blackboard.h"


BT_STATUS    CBehaviorTree::Tick(_float fTimeDelta)
{
    if (nullptr == m_pRoot)
        return BT_STATUS::FAILURE;

    return m_pRoot->Tick(m_pBB, fTimeDelta);
}

void CBehaviorTree::Reset()
{
    if (m_pRoot)
        m_pRoot->Reset();
}

CBehaviorTree* CBehaviorTree::Create(CBTNode* pRoot)
{
    CBehaviorTree* pInstance = new CBehaviorTree();
    pInstance->m_pRoot = pRoot;                
    pInstance->m_pBB = CBlackboard::Create();

    if (nullptr == pInstance->m_pBB)
    {
        MSG_BOX("Failed to Created : CBehaviorTree");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBehaviorTree::Free()
{
    Safe_Release(m_pRoot);
    Safe_Release(m_pBB);
}
