#include "NormalEnemy_Brain.h"

CNormalEnemy_Brain::CNormalEnemy_Brain()
{
}

HRESULT CNormalEnemy_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CNormalEnemy_Brain::Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{

}

CNormalEnemy_Brain* CNormalEnemy_Brain::Create(CMonster* pOwner)
{
	CNormalEnemy_Brain* pInstance = new CNormalEnemy_Brain();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CNormalEnemy_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CNormalEnemy_Brain::Free()
{
	__super::Free();
}
