#include "Split_Bush.h"
#include "GameContent_const.h"
#include "MeshEmitterCommon.h"

CSplit_Bush::CSplit_Bush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CSplit_Bush::CSplit_Bush(const CSplit_Bush& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSplit_Bush::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSplit_Bush::Initialize(void* pArg)
{
	EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSplit_Bush::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSplit_Bush::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSplit_Bush::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSplit_Bush::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CSplit_Bush::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = L"Prototype_Component_Model_BushLeafL";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("LeafL"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_BushLeafM";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("LeafM"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_BushLeafS";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("LeafS"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

CSplit_Bush* CSplit_Bush::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSplit_Bush* pInstance = new CSplit_Bush(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSplit_Bush");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSplit_Bush::Clone(void* pArg)
{
	CSplit_Bush* pInstance = new CSplit_Bush(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSplit_Bush");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSplit_Bush::Free()
{
	__super::Free();
}
