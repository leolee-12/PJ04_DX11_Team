#include "BTNode.h"

CBTAction* CBTAction::Create(TickFn fn, ResetFn rfn)
{
    CBTAction* pInstance = new CBTAction();
    pInstance->m_Fn = move(fn);
    pInstance->m_ResetFn = move(rfn);
    return pInstance;
}

void CBTAction::Free()
{
    __super::Free();
}

CBTCondition* CBTCondition::Create(CondFn fn)
{
    CBTCondition* pInstance = new CBTCondition();
    pInstance->m_Fn = move(fn);
    return pInstance;
}

void CBTCondition::Free()
{
    __super::Free();
}
