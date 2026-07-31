#include "HammerChargeGlow.h"

#include "GameContent_const.h"
#include "RectCommon.h"

CHammerChargeGlow::CHammerChargeGlow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CHammerChargeGlow::CHammerChargeGlow(const CHammerChargeGlow& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CHammerChargeGlow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHammerChargeGlow::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;
	return S_OK;
}

HRESULT CHammerChargeGlow::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tFlashDesc{};
	tFlashDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tFlashDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tFlashDesc.bUseTextureCom = true;
	tFlashDesc.iTextureLevel = m_iPrototypeLevel;
	tFlashDesc.wstrTextureTag = TEXTURE_FLASH_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"Flash", &tFlashDesc)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tRingDesc{};
	tRingDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tRingDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tRingDesc.bUseTextureCom = true;
	tRingDesc.iTextureLevel = m_iPrototypeLevel;
	tRingDesc.wstrTextureTag = TEXTURE_RING_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"GlowRing", &tRingDesc)))
		return E_FAIL;

	return S_OK;
}

CHammerChargeGlow* CHammerChargeGlow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHammerChargeGlow* pInstance = new CHammerChargeGlow(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CHammerChargeGlow");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CHammerChargeGlow::Clone(void* pArg)
{
	CHammerChargeGlow* pInstance = new CHammerChargeGlow(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CHammerChargeGlow");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CHammerChargeGlow::Free()
{
	__super::Free();
}
