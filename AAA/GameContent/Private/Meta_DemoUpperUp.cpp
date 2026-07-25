#include "Meta_DemoUpperUp.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshParticleCommon.h"

CMeta_DemoUpperUp::CMeta_DemoUpperUp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_DemoUpperUp::CMeta_DemoUpperUp(const CMeta_DemoUpperUp& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_DemoUpperUp::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_DemoUpperUp::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_DemoUpperUp::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_DemoUpperUp::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_DemoUpperUp::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_DemoUpperUp::Render()
{
	return __super::Render();
}

HRESULT CMeta_DemoUpperUp::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Flash1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Flash1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFlash1", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFlash2", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFlash3", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFlash4", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFlash5", &tDesc)))
		return E_FAIL;


	tDesc.iTextureLevel = Texture_Meta_HitFire1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_HitFire1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFire1", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFire2", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFire3", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMeta_DemoUpperUp* CMeta_DemoUpperUp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_DemoUpperUp* pInstance = new CMeta_DemoUpperUp(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_DemoUpperUp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_DemoUpperUp::Clone(void* pArg)
{
	CMeta_DemoUpperUp* pInstance = new CMeta_DemoUpperUp(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_DemoUpperUp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_DemoUpperUp::Free()
{
	__super::Free();
}
