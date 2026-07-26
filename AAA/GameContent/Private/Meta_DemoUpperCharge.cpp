#include "Meta_DemoUpperCharge.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshParticleCommon.h"

CMeta_DemoUpperCharge::CMeta_DemoUpperCharge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_DemoUpperCharge::CMeta_DemoUpperCharge(const CMeta_DemoUpperCharge& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_DemoUpperCharge::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_DemoUpperCharge::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_DemoUpperCharge::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_DemoUpperCharge::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_DemoUpperCharge::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_DemoUpperCharge::Render()
{
	return __super::Render();
}

HRESULT CMeta_DemoUpperCharge::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = false;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring01", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Flash1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Flash1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"ShineFlash", &tDesc)))
		return E_FAIL;

	tDesc.iTextureLevel = Texture_Meta_CircleFlash.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_CircleFlash.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"BigFlash", &tDesc)))
		return E_FAIL;

	tDesc.iTextureLevel = Texture_Meta_Line1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Line1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"LinePtcl", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMeta_DemoUpperCharge* CMeta_DemoUpperCharge::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_DemoUpperCharge* pInstance = new CMeta_DemoUpperCharge(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_DemoUpperCharge");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_DemoUpperCharge::Clone(void* pArg)
{
	CMeta_DemoUpperCharge* pInstance = new CMeta_DemoUpperCharge(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_DemoUpperCharge");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_DemoUpperCharge::Free()
{
	__super::Free();
}
