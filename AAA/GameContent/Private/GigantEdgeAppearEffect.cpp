#include "GigantEdgeAppearEffect.h"
#include "GameContent_const.h"

#include "RectParticleCommon.h"
#include "RectCommon.h"

CGigantEdgeAppearEffect::CGigantEdgeAppearEffect(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CGigantEdgeAppearEffect::CGigantEdgeAppearEffect(
	const CGigantEdgeAppearEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CGigantEdgeAppearEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGigantEdgeAppearEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CGigantEdgeAppearEffect::Priority_Update(
	_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGigantEdgeAppearEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CGigantEdgeAppearEffect::Late_Update(
	_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CGigantEdgeAppearEffect::Render()
{
	return __super::Render();
}

HRESULT CGigantEdgeAppearEffect::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tCoreDesc{};
	tCoreDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tCoreDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tCoreDesc.bUseTextureCom = true;
	tCoreDesc.iTextureLevel =
		Texture_GE_CircleQuarter.iLevelID;
	tCoreDesc.wstrTextureTag =
		Texture_GE_CircleQuarter.szProtoTag;
	tCoreDesc.bUseMaskCom = false;
	tCoreDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CRectCommon::PROTOTYPE_TAG,
		L"Core", &tCoreDesc)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tGlowDesc{};
	tGlowDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tGlowDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tGlowDesc.bUseTextureCom = true;
	tGlowDesc.iTextureLevel =
		Texture_GE_CircleGlow.iLevelID;
	tGlowDesc.wstrTextureTag =
		Texture_GE_CircleGlow.szProtoTag;
	tGlowDesc.bUseMaskCom = false;
	tGlowDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CRectCommon::PROTOTYPE_TAG,
		L"Glow", &tGlowDesc)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tRadiationDesc{};
	tRadiationDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tRadiationDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tRadiationDesc.bUseTextureCom = true;
	tRadiationDesc.iTextureLevel =
		Texture_GE_Radiation.iLevelID;
	tRadiationDesc.wstrTextureTag =
		Texture_GE_Radiation.szProtoTag;
	tRadiationDesc.bUseMaskCom = false;
	tRadiationDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CRectCommon::PROTOTYPE_TAG,
		L"Radiation", &tRadiationDesc)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tRayNeedlesDesc{};
	tRayNeedlesDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tRayNeedlesDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tRayNeedlesDesc.bUseTextureCom = true;
	tRayNeedlesDesc.iTextureLevel =
		Texture_GE_RingRay.iLevelID;
	tRayNeedlesDesc.wstrTextureTag =
		Texture_GE_RingRay.szProtoTag;
	tRayNeedlesDesc.bUseMaskCom = false;
	tRayNeedlesDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CRectCommon::PROTOTYPE_TAG,
		L"RayNeedles", &tRayNeedlesDesc)))
		return E_FAIL;

	CRectParticleCommon::RECT_PARTICLE_COMMON_DESC
		tPetalDesc{};
	tPetalDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tPetalDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tPetalDesc.bUseTextureCom = true;
	tPetalDesc.iTextureLevel =
		Texture_GE_Leaf.iLevelID;
	tPetalDesc.wstrTextureTag =
		Texture_GE_Leaf.szProtoTag;
	tPetalDesc.bUseMaskCom = false;
	tPetalDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CRectParticleCommon::PROTOTYPE_TAG,
		L"Petal", &tPetalDesc)))
		return E_FAIL;

	return S_OK;
}

CGigantEdgeAppearEffect*
CGigantEdgeAppearEffect::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CGigantEdgeAppearEffect* pInstance =
		new CGigantEdgeAppearEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(
			"Failed to Created: CGigantEdgeAppearEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGigantEdgeAppearEffect::Clone(void* pArg)
{
	CGigantEdgeAppearEffect* pInstance =
		new CGigantEdgeAppearEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(
			"Failed to Cloned: CGigantEdgeAppearEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGigantEdgeAppearEffect::Free()
{
	__super::Free();
}
