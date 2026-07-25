#include "Meta_UpperCharge.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshParticleCommon.h"

CMeta_UpperCharge::CMeta_UpperCharge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_UpperCharge::CMeta_UpperCharge(const CMeta_UpperCharge& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_UpperCharge::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_UpperCharge::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_UpperCharge::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_UpperCharge::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_UpperCharge::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_UpperCharge::Render()
{
	return __super::Render();
}

HRESULT CMeta_UpperCharge::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = false;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring01", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring02", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring03", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring04", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring05", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring06", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring07", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring08", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring09", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring10", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Flash1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Flash1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"ShineFlash", &tDesc)))
		return E_FAIL;

	tDesc.iTextureLevel = Texture_Meta_Shine1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Shine1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Shine", &tDesc)))
		return E_FAIL;

	tDesc.iTextureLevel = Texture_Meta_CircleFlash.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_CircleFlash.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"BigFlash", &tDesc)))
		return E_FAIL;

	tDesc.iTextureLevel = Texture_Meta_Line1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Line1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"LinePtcl", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = MODEL_PROTO_TAG_THUNDER;
	tDesc.bUseTextureCom = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"ThunderPtcl", &tDesc)))
		return E_FAIL;


	return S_OK;
}

CMeta_UpperCharge* CMeta_UpperCharge::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_UpperCharge* pInstance = new CMeta_UpperCharge(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_UpperCharge");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_UpperCharge::Clone(void* pArg)
{
	CMeta_UpperCharge* pInstance = new CMeta_UpperCharge(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_UpperCharge");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_UpperCharge::Free()
{
	__super::Free();
}
