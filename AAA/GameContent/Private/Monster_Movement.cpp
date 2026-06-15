#include "Monster_Movement.h"

CMonster_Movement::CMonster_Movement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMovement{ pDevice, pContext }
{
}

CMonster_Movement::CMonster_Movement(const CMonster_Movement& Prototype)
	: CMovement (Prototype)
{
}

HRESULT CMonster_Movement::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMonster_Movement::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

CMonster_Movement* CMonster_Movement::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonster_Movement* pInstance = new CMonster_Movement(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster_Movement");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMonster_Movement::Clone(void* pArg)
{
	CMonster_Movement* pInstance = new CMonster_Movement(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_Movement");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_Movement::Free()
{
	__super::Free();
}
