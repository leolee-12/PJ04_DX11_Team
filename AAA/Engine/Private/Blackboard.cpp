#include "Blackboard.h"

CBlackboard* CBlackboard::Create()
{
    return new CBlackboard();
}

void CBlackboard::Free()
{
    __super::Free();
}
