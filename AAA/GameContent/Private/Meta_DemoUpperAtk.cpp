#include "Meta_DemoUpperAtk.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshParticleCommon.h"

CMeta_DemoUpperAtk::CMeta_DemoUpperAtk(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_DemoUpperAtk::CMeta_DemoUpperAtk(const CMeta_DemoUpperAtk& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_DemoUpperAtk::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_DemoUpperAtk::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_DemoUpperAtk::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_DemoUpperAtk::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_DemoUpperAtk::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_DemoUpperAtk::Render()
{
	return __super::Render();
}

HRESULT CMeta_DemoUpperAtk::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Ring1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Ring1.szProtoTag;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring", &tDesc)))
		return E_FAIL;


	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	tDesc.iTextureLevel = Texture_Meta_HitFire1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_HitFire1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFire1", &tDesc)))
		return E_FAIL;

	tDesc.iTextureLevel = Texture_Meta_HitFire2.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_HitFire2.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFire2", &tDesc)))
		return E_FAIL;

	tDesc.iTextureLevel = Texture_Meta_Line2.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Line2.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitLine1", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitLine2", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"LinePtcl", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMeta_DemoUpperAtk* CMeta_DemoUpperAtk::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_DemoUpperAtk* pInstance = new CMeta_DemoUpperAtk(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_DemoUpperAtk");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_DemoUpperAtk::Clone(void* pArg)
{
	CMeta_DemoUpperAtk* pInstance = new CMeta_DemoUpperAtk(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_DemoUpperAtk");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_DemoUpperAtk::Free()
{
	__super::Free();
}
