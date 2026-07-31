#include "CrashChargeEffect.h"

#include "DistortionCommon.h"
#include "GameContent_const.h"
#include "MeshCommon.h"
#include "RectCommon.h"
#include "RectEmitterCommon.h"

namespace
{
	CMeshCommon::MESH_COMMON_DESC Make_MeshDesc(
		_uint iLevel,
		const _tchar* szModelTag,
		const _tchar* szTextureTag,
		const _tchar* szMaskTag)
	{
		CMeshCommon::MESH_COMMON_DESC tDesc{};
		tDesc.iModelLevel = iLevel;
		tDesc.wstrModelTag = szModelTag;
		tDesc.bUseDiffuseTexture = false;
		tDesc.bUseNormalTexture = false;
		tDesc.bUseMRATexture = false;
		tDesc.bUseUnknownTexture = false;
		tDesc.bUseTextureCom = true;
		tDesc.iTextureLevel = iLevel;
		tDesc.wstrTextureTag = szTextureTag;
		tDesc.bUseMaskCom = true;
		tDesc.iMaskLevel = iLevel;
		tDesc.wstrMaskTag = szMaskTag;
		tDesc.bCustomShader = false;
		return tDesc;
	}

	CDistortionCommon::DISTORTION_COMMON_DESC Make_DistortionDesc(
		_uint iLevel,
		const _tchar* szModelTag,
		const _tchar* szTextureTag,
		const _tchar* szMaskTag)
	{
		CDistortionCommon::DISTORTION_COMMON_DESC tDesc{};
		tDesc.iModelLevel = iLevel;
		tDesc.wstrModelTag = szModelTag;
		tDesc.bUseTextureCom = true;
		tDesc.iTextureLevel = iLevel;
		tDesc.wstrTextureTag = szTextureTag;
		tDesc.bUseMaskCom = true;
		tDesc.iMaskLevel = iLevel;
		tDesc.wstrMaskTag = szMaskTag;
		tDesc.bCustomShader = false;
		return tDesc;
	}

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC Make_RectEmitterDesc(
		_uint iLevel,
		const _tchar* szTextureTag)
	{
		CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tDesc{};
		tDesc.iVIBufferLevel = VI_Rect.iLevelID;
		tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
		tDesc.bUseTextureCom = true;
		tDesc.iTextureLevel = iLevel;
		tDesc.wstrTextureTag = szTextureTag;
		tDesc.bUseMaskCom = false;
		tDesc.bCustomShader = false;
		return tDesc;
	}

	CRectCommon::RECT_COMMON_DESC Make_RectDesc(_uint iLevel, const _tchar* szTextureTag)
	{
		CRectCommon::RECT_COMMON_DESC tDesc{};
		tDesc.iVIBufferLevel = VI_Rect.iLevelID;
		tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
		tDesc.bUseTextureCom = true;
		tDesc.iTextureLevel = iLevel;
		tDesc.wstrTextureTag = szTextureTag;
		tDesc.bUseMaskCom = false;
		tDesc.bCustomShader = false;
		return tDesc;
	}
}

CCrashChargeEffect::CCrashChargeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CCrashChargeEffect::CCrashChargeEffect(const CCrashChargeEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CCrashChargeEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCrashChargeEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCrashChargeEffect::Ready_EffectPartObjects()
{
	CDistortionCommon::DISTORTION_COMMON_DESC tWarp = Make_DistortionDesc(
		m_iPrototypeLevel, MODEL_CIRCLE_TAG, TEXTURE_WARP_CHARGE_TAG, TEXTURE_CIRCLE06_TAG);
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CDistortionCommon::PROTOTYPE_TAG, L"Warp", &tWarp)))
		return E_FAIL;

	CDistortionCommon::DISTORTION_COMMON_DESC tWarpInner = Make_DistortionDesc(
		m_iPrototypeLevel, MODEL_CIRCLE_TAG, TEXTURE_WARP_CHARGE_TAG, TEXTURE_CIRCLE01_TAG);
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CDistortionCommon::PROTOTYPE_TAG, L"WarpInnerDistortion", &tWarpInner)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tCoreGlow = Make_RectDesc(m_iPrototypeLevel, TEXTURE_CIRCLE06_TAG);
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"CoreGlow", &tCoreGlow)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tCoreFlare = Make_RectDesc(m_iPrototypeLevel, TEXTURE_TWINKLE03_TAG);
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"CoreFlareWide", &tCoreFlare)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"CoreFlareCross", &tCoreFlare)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC tCenter = Make_MeshDesc(
		m_iPrototypeLevel, MODEL_CIRCLE_TAG, TEXTURE_CIRCLE06_TAG, TEXTURE_CIRCLE01_TAG);
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Center", &tCenter)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC tCenterOuter = Make_MeshDesc(
		m_iPrototypeLevel, MODEL_CIRCLE_TAG, TEXTURE_CIRCLE01_TAG, TEXTURE_TWINKLE03_TAG);
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"CenterOuter", &tCenterOuter)))
		return E_FAIL;

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tCenterTwinkle =
		Make_RectEmitterDesc(m_iPrototypeLevel, TEXTURE_TWINKLE03_TAG);
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"CenterTwinkle", &tCenterTwinkle)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"CenterTwinkleOuter", &tCenterTwinkle)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC tWarpAura = Make_MeshDesc(
		m_iPrototypeLevel, MODEL_CIRCLE_TAG, TEXTURE_CIRCLE06_TAG, TEXTURE_SCROLL14_TAG);
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"WarpAura", &tWarpAura)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC tWarpAuraOuter = Make_MeshDesc(
		m_iPrototypeLevel, MODEL_CIRCLE_TAG, TEXTURE_CIRCLE01_TAG, TEXTURE_CIRCLE06_TAG);
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"WarpAuraOuter", &tWarpAuraOuter)))
		return E_FAIL;

	CDistortionCommon::DISTORTION_COMMON_DESC tWarpAuraDistortion = Make_DistortionDesc(
		m_iPrototypeLevel, MODEL_CIRCLE_TAG, TEXTURE_INDIRECT3_TAG, TEXTURE_CIRCLE01_TAG);
	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel, CDistortionCommon::PROTOTYPE_TAG, L"WarpAuraDistortion", &tWarpAuraDistortion)))
		return E_FAIL;

	return S_OK;
}

CCrashChargeEffect* CCrashChargeEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCrashChargeEffect* pInstance = new CCrashChargeEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CCrashChargeEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCrashChargeEffect::Clone(void* pArg)
{
	CCrashChargeEffect* pInstance = new CCrashChargeEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCrashChargeEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCrashChargeEffect::Free()
{
	__super::Free();
}
