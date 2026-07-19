#include "Bouncy_Brain.h"
#include "Bouncy.h"

CBouncy_Brain::CBouncy_Brain()
{
}

HRESULT CBouncy_Brain::Initialize(CMonster* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CBouncy_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    UNREFERENCED_PARAMETER(BlackBoard);
    UNREFERENCED_PARAMETER(fTimeDelta);
}

CBouncy_Brain* CBouncy_Brain::Create(CMonster* pOwner)
{
    CBouncy_Brain* pInstance = new CBouncy_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CBouncy_Brain");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBouncy_Brain::Free()
{
    __super::Free();
}
