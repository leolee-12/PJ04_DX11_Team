#include "HammerImpactGround.h"

#include "DistortionCommon.h"
#include "GameContent_const.h"
#include "MeshCommon.h"
#include "RectCommon.h"

CHammerImpactGround::CHammerImpactGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CHammerImpactGround::CHammerImpactGround(const CHammerImpactGround& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CHammerImpactGround::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHammerImpactGround::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CHammerImpactGround::Ready_EffectPartObjects()
{
	CDistortionCommon::DISTORTION_COMMON_DESC WarpDesc{};
	WarpDesc.iModelLevel = m_iPrototypeLevel;
	WarpDesc.wstrModelTag = MODEL_RING_TAG;
	WarpDesc.bUseTextureCom = true;
	WarpDesc.iTextureLevel = m_iPrototypeLevel;
	WarpDesc.wstrTextureTag = TEXTURE_WARP_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CDistortionCommon::PROTOTYPE_TAG, L"RingWarp", &WarpDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC RingDesc{};
	RingDesc.iModelLevel = m_iPrototypeLevel;
	RingDesc.wstrModelTag = MODEL_RING_TAG;
	RingDesc.bUseDiffuseTexture = false;
	RingDesc.bUseNormalTexture = false;
	RingDesc.bUseMRATexture = false;
	RingDesc.bUseUnknownTexture = false;
	RingDesc.bUseTextureCom = true;
	RingDesc.iTextureLevel = m_iPrototypeLevel;
	RingDesc.wstrTextureTag = TEXTURE_RING_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"RingColor", &RingDesc)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC StarDesc{};
	StarDesc.iVIBufferLevel = VI_Rect.iLevelID;
	StarDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	StarDesc.bUseTextureCom = true;
	StarDesc.iTextureLevel = m_iPrototypeLevel;
	StarDesc.wstrTextureTag = TEXTURE_STAR_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"GroundStar", &StarDesc)))
		return E_FAIL;

	return S_OK;
}

CHammerImpactGround* CHammerImpactGround::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHammerImpactGround* pInstance = new CHammerImpactGround(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CHammerImpactGround");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHammerImpactGround::Clone(void* pArg)
{
	CHammerImpactGround* pInstance = new CHammerImpactGround(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CHammerImpactGround");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHammerImpactGround::Free()
{
	__super::Free();
}
