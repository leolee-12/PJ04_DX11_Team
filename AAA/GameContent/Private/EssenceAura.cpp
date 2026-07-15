#include "EssenceAura.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "RectCommon.h"
#include "RectEmitterCommon.h"
#include "RectParticleCommon.h"

namespace
{
	CRectCommon::RECT_COMMON_DESC Make_RectDesc(const TEXTURE_DESC& tex)
	{
		CRectCommon::RECT_COMMON_DESC tRect{};
		tRect.iVIBufferLevel = VI_Rect.iLevelID;
		tRect.wstrVIBufferTag = VI_Rect.szProtoTag;

		tRect.bUseTextureCom = true;
		tRect.iTextureLevel = tex.iLevelID;
		tRect.wstrTextureTag = tex.szProtoTag;

		tRect.bUseMaskCom = false;
		tRect.bCustomShader = false;

		return tRect;
	}
}

CEssenceAura::CEssenceAura(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CEssenceAura::CEssenceAura(const CEssenceAura& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CEssenceAura::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEssenceAura::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CEssenceAura::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CEssenceAura::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CEssenceAura::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CEssenceAura::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEssenceAura::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tRing = Make_RectDesc(Texture_CommonRing01);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"RingHalo", &tRing)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tStar = Make_RectDesc(Texture_Star2D);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"OribitStar00", &tStar)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"OribitStar01", &tStar)))
		return E_FAIL;

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tStarEmit{};
	tStarEmit.iVIBufferLevel = VI_Rect.iLevelID;
	tStarEmit.wstrVIBufferTag = VI_Rect.szProtoTag;
	tStarEmit.bUseTextureCom = true;
	tStarEmit.iTextureLevel = Texture_Star2D.iLevelID;
	tStarEmit.wstrTextureTag = Texture_Star2D.szProtoTag;
	tStarEmit.bUseMaskCom = false;
	tStarEmit.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"StarMilkyWay00", &tStarEmit)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"StarMilkyWay01", &tStarEmit)))
		return E_FAIL;

	CRectParticleCommon::RECT_PARTICLE_COMMON_DESC tSparkle{};
	tSparkle.iVIBufferLevel = VI_Rect.iLevelID;
	tSparkle.wstrVIBufferTag = VI_Rect.szProtoTag;
	tSparkle.bUseTextureCom = true;
	tSparkle.iTextureLevel = Texture_CommonSparkle01.iLevelID;
	tSparkle.wstrTextureTag = Texture_CommonSparkle01.szProtoTag;
	tSparkle.bUseMaskCom = false;
	tSparkle.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectParticleCommon::PROTOTYPE_TAG, L"Sparkle", &tSparkle)))
		return E_FAIL;

	return S_OK;
}

CEssenceAura* CEssenceAura::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEssenceAura* pInstance = new CEssenceAura(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEssenceAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEssenceAura::Clone(void* pArg)
{
	CEssenceAura* pInstance = new CEssenceAura(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEssenceAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEssenceAura::Free()
{
	__super::Free();
}
