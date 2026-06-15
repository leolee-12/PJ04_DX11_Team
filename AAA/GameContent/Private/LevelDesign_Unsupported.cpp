#include "LevelDesign_Unsupported.h"

NS_BEGIN(Client)

CLevelDesign_Unsupported::CLevelDesign_Unsupported(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Unsupported::CLevelDesign_Unsupported(const CLevelDesign_Unsupported& Prototype)
	: CLevelDesignObject(Prototype)
{
}

HRESULT CLevelDesign_Unsupported::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Unsupported::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Unsupported::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

CLevelDesign_Unsupported* CLevelDesign_Unsupported::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Unsupported* pInstance = new CLevelDesign_Unsupported(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Unsupported");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Unsupported::Clone(void* pArg)
{
	CLevelDesign_Unsupported* pInstance = new CLevelDesign_Unsupported(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Unsupported");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Unsupported::Free()
{
	__super::Free();
}

NS_END