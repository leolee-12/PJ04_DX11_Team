#include "FlowerPetals.h"
#include "GameContent_const.h"
#include "MeshEmitterCommon.h"

CFlowerPetals::CFlowerPetals(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CFlowerPetals::CFlowerPetals(const CFlowerPetals& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CFlowerPetals::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CFlowerPetals::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = L"Prototype_Component_Model_Flower";
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Flower_0"), &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Flower_1"), &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Flower_2"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

CFlowerPetals* CFlowerPetals::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFlowerPetals* pInstance = new CFlowerPetals(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CFlowerPetals");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFlowerPetals::Clone(void* pArg)
{
	CFlowerPetals* pInstance = new CFlowerPetals(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CFlowerPetals");
		Safe_Release(pInstance);
	}

	return pInstance;
}