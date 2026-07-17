#include "PickUpEffect.h"
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

CPickUpEffect::CPickUpEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CPickUpEffect::CPickUpEffect(const CPickUpEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CPickUpEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPickUpEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CPickUpEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CPickUpEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CPickUpEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CPickUpEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CPickUpEffect::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tRing = Make_RectDesc(Texture_CommonRing01);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"RingHalo", &tRing)))
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

CPickUpEffect* CPickUpEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPickUpEffect* pInstance = new CPickUpEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPickUpEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPickUpEffect::Clone(void* pArg)
{
	CPickUpEffect* pInstance = new CPickUpEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPickUpEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPickUpEffect::Free()
{
	__super::Free();
}
