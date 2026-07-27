#include "SwordHitEffect.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshParticleCommon.h"

CSwordHitEffect::CSwordHitEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSwordHitEffect::CSwordHitEffect(const CSwordHitEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSwordHitEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSwordHitEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSwordHitEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSwordHitEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSwordHitEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSwordHitEffect::Render()
{
	return __super::Render();
}

HRESULT CSwordHitEffect::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = true;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	tDesc.iTextureLevel = Texture_Common_Flash02.iLevelID;
	tDesc.wstrTextureTag = Texture_Common_Flash02.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"HitFire1", &tDesc)))
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

CSwordHitEffect* CSwordHitEffect::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CSwordHitEffect* pInstance = new CSwordHitEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSwordHitEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwordHitEffect::Clone(void* pArg)
{
	CSwordHitEffect* pInstance = new CSwordHitEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSwordHitEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwordHitEffect::Free()
{
	__super::Free();
}
