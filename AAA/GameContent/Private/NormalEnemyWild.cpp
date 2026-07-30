#include "NormalEnemyWild.h"
#include "GameInstance.h"

#include "NormalEnemyWild_Body.h"

CNormalEnemyWild::CNormalEnemyWild(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CNormalEnemy { pDevice, pContext }
{
}

CNormalEnemyWild::CNormalEnemyWild(const CNormalEnemyWild& Prototype)
	: CNormalEnemy (Prototype)
{
}

HRESULT CNormalEnemyWild::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_fCullDist = 175.f;

	return S_OK;
}

HRESULT CNormalEnemyWild::Ready_PartObjects()
{
	m_pBody = Add_MonsterPart<CNormalEnemy_Body>(CNormalEnemyWild_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

CNormalEnemyWild* CNormalEnemyWild::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNormalEnemyWild* pInstance	= new CNormalEnemyWild(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CNormalEnemyWild");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CNormalEnemyWild::Clone(void* pArg)
{
	CNormalEnemyWild* pInstance	= new CNormalEnemyWild(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CNormalEnemyWild");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CNormalEnemyWild::Free()
{
	__super::Free();
}
