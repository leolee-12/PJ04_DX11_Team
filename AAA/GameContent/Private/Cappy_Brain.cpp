#include "Cappy_Brain.h"

HRESULT CCappy_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
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
