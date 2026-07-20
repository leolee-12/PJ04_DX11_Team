#include "Split_Coaster.h"
#include "GameContent_const.h"
#include "MeshEmitterCommon.h"

CSplit_Coaster::CSplit_Coaster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CSplit_Coaster::CSplit_Coaster(const CSplit_Coaster& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSplit_Coaster::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSplit_Coaster::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Coaster_Bar";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Coaster_Bar"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Coaster_Jet";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Coaster_Jet"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Coaster_Tip01L";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Coaster_Tip01L"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Coaster_Tip02L";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Coaster_Tip02L"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Coaster_Tire";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Coaster_Tire"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Coaster_WingA";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Coaster_WingA"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Coaster_WingB";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Coaster_WingB"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = Model_SmokeSphereOriginal.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Smoke"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

CSplit_Coaster* CSplit_Coaster::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSplit_Coaster* pInstance = new CSplit_Coaster(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSplit_Coaster");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSplit_Coaster::Clone(void* pArg)
{
	CSplit_Coaster* pInstance = new CSplit_Coaster(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSplit_Coaster");
		Safe_Release(pInstance);
	}

	return pInstance;
}