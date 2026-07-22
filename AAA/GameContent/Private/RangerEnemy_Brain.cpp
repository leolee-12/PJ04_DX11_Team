#include "RangerEnemy_Brain.h"
#include "Monster.h"

CRangerEnemy_Brain::CRangerEnemy_Brain()
{
}

HRESULT CRangerEnemy_Brain::Initialize(CMonster* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

CRangerEnemy_Brain* CRangerEnemy_Brain::Create(CMonster* pOwner)
{
    CRangerEnemy_Brain* pInstance = new CRangerEnemy_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CRangerEnemy_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRangerEnemy_Brain::Free()
{
    __super::Free();
}
