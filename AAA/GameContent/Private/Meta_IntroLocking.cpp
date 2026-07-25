#include "Meta_IntroLocking.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshParticleCommon.h"

CMeta_IntroLocking::CMeta_IntroLocking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_IntroLocking::CMeta_IntroLocking(const CMeta_IntroLocking& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_IntroLocking::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_IntroLocking::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_IntroLocking::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_IntroLocking::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_IntroLocking::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_IntroLocking::Render()
{
	return __super::Render();
}

HRESULT CMeta_IntroLocking::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = false;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring1", &tDesc)))
		return E_FAIL;

	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Ring1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Ring1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring2", &tDesc)))
		return E_FAIL;

	tDesc.bUseTextureCom = false;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Flash1", &tDesc)))
		return E_FAIL;

	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Flash1.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Flash1.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Flash2", &tDesc)))
		return E_FAIL;

	tDesc.bUseTextureCom = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"FlashPtcl", &tDesc)))
		return E_FAIL;
	return S_OK;
}

CMeta_IntroLocking* CMeta_IntroLocking::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_IntroLocking* pInstance = new CMeta_IntroLocking(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_IntroLocking");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_IntroLocking::Clone(void* pArg)
{
	CMeta_IntroLocking* pInstance = new CMeta_IntroLocking(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_IntroLocking");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_IntroLocking::Free()
{
	__super::Free();
}
