#include "Cappy_Brain.h"

void CCappy_Brain::Decide(const MONSTER_BLACKBOARD&, _float)
{

}

HRESULT CCappy_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CCappy_Brain::Decide_Internal(const MONSTER_BLACKBOARD&, _float)
{

}

CCappy_Brain* CCappy_Brain::Create(CMonster* pOwner)
{
	CCappy_Brain* pInstance = new CCappy_Brain();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CCappy_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy_Brain::Free()
{
	__super::Free();
}
