#include "SirKibble_Brain.h"
#include "Monster.h"

CSirKibble_Brain::CSirKibble_Brain()
{
}

HRESULT CSirKibble_Brain::Initialize(CMonster* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

CSirKibble_Brain* CSirKibble_Brain::Create(CMonster* pOwner)
{
    CSirKibble_Brain* pInstance = new CSirKibble_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CSirKibble_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSirKibble_Brain::Free()
{
    __super::Free();
}
