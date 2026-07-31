#include "CrashEffect.h"

#include "DistortionCommon.h"
#include "GameContent_const.h"
#include "MeshCommon.h"
#include "MeshEmitterCommon.h"
#include "RectCommon.h"
#include "RectEmitterCommon.h"

namespace
{
	CMeshCommon::MESH_COMMON_DESC Make_MeshDesc(_uint iLevel, const _tchar* szModelTag, _bool bUseDiffuse, _bool bUseUnknown)
	{
		CMeshCommon::MESH_COMMON_DESC tDesc{};
		tDesc.iModelLevel = iLevel;
		tDesc.wstrModelTag = szModelTag;
		tDesc.bUseDiffuseTexture = bUseDiffuse;
		tDesc.bUseNormalTexture = false;
		tDesc.bUseMRATexture = false;
		tDesc.bUseUnknownTexture = bUseUnknown;
		tDesc.bCustomShader = false;
		return tDesc;
	}

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC Make_MeshEmitterDesc(_uint iLevel, const _tchar* szModelTag, _bool bUseDiffuse, _bool bUseUnknown)
	{
		CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
		tDesc.iModelLevel = iLevel;
		tDesc.wstrModelTag = szModelTag;
		tDesc.bUseDiffuseTexture = bUseDiffuse;
		tDesc.bUseNormalTexture = false;
		tDesc.bUseMRATexture = false;
		tDesc.bUseUnknownTexture = bUseUnknown;
		tDesc.bCustomShader = false;
		return tDesc;
	}

	CRectCommon::RECT_COMMON_DESC Make_RectDesc(_uint iTextureLevel, const _tchar* szTextureTag)
	{
		CRectCommon::RECT_COMMON_DESC tDesc{};
		tDesc.iVIBufferLevel = VI_Rect.iLevelID;
		tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
		tDesc.bUseTextureCom = true;
		tDesc.iTextureLevel = iTextureLevel;
		tDesc.wstrTextureTag = szTextureTag;
		tDesc.bCustomShader = false;
		return tDesc;
	}

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC Make_RectEmitterDesc(_uint iTextureLevel, const _tchar* szTextureTag)
	{
		CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tDesc{};
		tDesc.iVIBufferLevel = VI_Rect.iLevelID;
		tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
		tDesc.bUseTextureCom = true;
		tDesc.iTextureLevel = iTextureLevel;
		tDesc.wstrTextureTag = szTextureTag;
		tDesc.bCustomShader = false;
		return tDesc;
	}
}

CCrashEffect::CCrashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CCrashEffect::CCrashEffect(const CCrashEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CCrashEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCrashEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CCrashEffect::Priority_Update(_float fTimeDelta)
{
	fTimeDelta = Resolve_TimeDelta(fTimeDelta);
	__super::Priority_Update(fTimeDelta);
}

void CCrashEffect::Update(_float fTimeDelta)
{
	fTimeDelta = Resolve_TimeDelta(fTimeDelta);
	__super::Update(fTimeDelta);
}

void CCrashEffect::Late_Update(_float fTimeDelta)
{
	fTimeDelta = Resolve_TimeDelta(fTimeDelta);
	__super::Late_Update(fTimeDelta);
}

HRESULT CCrashEffect::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tGroundSmoke =
		Make_MeshEmitterDesc(m_iPrototypeLevel, MODEL_GROUND_SMOKE_TAG, false, true);
	tGroundSmoke.bUseTextureCom = true;
	tGroundSmoke.bCustomShader = true;
	tGroundSmoke.iShaderLevel = Shader_SpecialEffect.iLevelID;
	tGroundSmoke.wstrShaderTag = Shader_SpecialEffect.szProtoTag;
	tGroundSmoke.iTextureLevel = m_iPrototypeLevel;
	tGroundSmoke.wstrTextureTag = TEXTURE_SLASH_TAG;
	tGroundSmoke.bUseMaskCom = true;
	tGroundSmoke.iMaskLevel = m_iPrototypeLevel;
	tGroundSmoke.wstrMaskTag = TEXTURE_WAVE_TAG;

	for (_uint i = 0; i < 10; ++i)
	{
		_wstring strPartTag = L"GroundSmoke";
		if (i != 0)
		{
			strPartTag += L"_";
			strPartTag += std::to_wstring(i);
		}

		if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, strPartTag, &tGroundSmoke)))
			return E_FAIL;
	}

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tFlame =
		Make_MeshEmitterDesc(m_iPrototypeLevel, MODEL_SPHERE_TAG, true, false);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"FlameA", &tFlame)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"FlameA_Copy1", &tFlame)))
		return E_FAIL;

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tRingShock =
		Make_MeshEmitterDesc(m_iPrototypeLevel, MODEL_RING_TAG, false, true);
	tRingShock.bUseTextureCom = true;
	tRingShock.iTextureLevel = m_iPrototypeLevel;
	tRingShock.wstrTextureTag = TEXTURE_SHINE_CIRCLE_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"RingShock", &tRingShock)))
		return E_FAIL;

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tLine =
		Make_RectEmitterDesc(m_iPrototypeLevel, TEXTURE_CIRCLE01_TAG);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"Line", &tLine)))
		return E_FAIL;

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tStars =
		Make_RectEmitterDesc(m_iPrototypeLevel, TEXTURE_STAR00_TAG);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"Stars", &tStars)))
		return E_FAIL;

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tCircles =
		Make_RectEmitterDesc(m_iPrototypeLevel, TEXTURE_CIRCLE02_TAG);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"Stars_Copy1_Copy1", &tCircles)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tCenter =
		Make_RectDesc(m_iPrototypeLevel, TEXTURE_SHINE_CIRCLE_TAG);
	tCenter.bUseMaskCom = true;
	tCenter.iMaskLevel = m_iPrototypeLevel;
	tCenter.wstrMaskTag = TEXTURE_CIRCLE06_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"Center", &tCenter)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC tShine =
		Make_MeshDesc(m_iPrototypeLevel, MODEL_CIRCLE_TAG, false, true);
	tShine.bUseTextureCom = true;
	tShine.iTextureLevel = m_iPrototypeLevel;
	tShine.wstrTextureTag = TEXTURE_RING04_TAG;
	tShine.bUseMaskCom = true;
	tShine.iMaskLevel = m_iPrototypeLevel;
	tShine.wstrMaskTag = TEXTURE_CIRCLE07_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Shine", &tShine)))
		return E_FAIL;

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tShine2 =
		Make_MeshEmitterDesc(m_iPrototypeLevel, MODEL_CIRCLE_TAG, false, true);
	tShine2.bUseTextureCom = true;
	tShine2.iTextureLevel = m_iPrototypeLevel;
	tShine2.wstrTextureTag = TEXTURE_CIRCLE06_TAG;
	tShine2.bUseMaskCom = true;
	tShine2.iMaskLevel = m_iPrototypeLevel;
	tShine2.wstrMaskTag = TEXTURE_CIRCLE_GLOW_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Shine2", &tShine2)))
		return E_FAIL;

	CDistortionCommon::DISTORTION_COMMON_DESC tWarp{};
	tWarp.iModelLevel = m_iPrototypeLevel;
	tWarp.wstrModelTag = MODEL_CIRCLE_TAG;
	tWarp.bUseTextureCom = true;
	tWarp.iTextureLevel = m_iPrototypeLevel;
	tWarp.wstrTextureTag = TEXTURE_WARP_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CDistortionCommon::PROTOTYPE_TAG, L"Warp", &tWarp)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tScreen =
		Make_RectDesc(m_iPrototypeLevel, TEXTURE_CIRCLE02_TAG);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"Screen", &tScreen)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tAfter =
		Make_RectDesc(m_iPrototypeLevel, TEXTURE_CIRCLE02_TAG);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"After", &tAfter)))
		return E_FAIL;

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tStar =
		Make_RectEmitterDesc(m_iPrototypeLevel, TEXTURE_STAR03_TAG);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"Star", &tStar)))
		return E_FAIL;

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tStone =
		Make_MeshEmitterDesc(m_iPrototypeLevel, MODEL_BREAKABLE_ROCK_TAG, true, false);
	tStone.bUseNormalTexture = true;
	tStone.bCustomShader = true;
	tStone.iShaderLevel = Shader_EffectRock.iLevelID;
	tStone.wstrShaderTag = Shader_EffectRock.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Stone", &tStone)))
		return E_FAIL;

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tThunder =
		Make_MeshEmitterDesc(m_iPrototypeLevel, MODEL_THUNDER_SPIRAL_TAG, false, true);
	tThunder.bUseTextureCom = true;
	tThunder.iTextureLevel = m_iPrototypeLevel;
	tThunder.wstrTextureTag = TEXTURE_THUNDER_STRAIGHT_TAG;
	tThunder.bUseMaskCom = true;
	tThunder.iMaskLevel = m_iPrototypeLevel;
	tThunder.wstrMaskTag = TEXTURE_INDIRECT_NORMAL_TAG;
	tThunder.bCustomShader = true;
	tThunder.iShaderLevel = Shader_SpecialEffect.iLevelID;
	tThunder.wstrShaderTag = Shader_SpecialEffect.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Thunder", &tThunder)))
		return E_FAIL;

	return S_OK;
}

_float CCrashEffect::Resolve_TimeDelta(_float fTimeDelta)
{
	return m_pGameInstance_Proxy->Get_RawTimeDelta(TEXT("Timer_60"));
}

CCrashEffect* CCrashEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCrashEffect* pInstance = new CCrashEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CCrashEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCrashEffect::Clone(void* pArg)
{
	CCrashEffect* pInstance = new CCrashEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCrashEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCrashEffect::Free()
{
	__super::Free();
}
