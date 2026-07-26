#include "HammerSwing.h"

#include "DistortionCommon.h"
#include "GameContent_const.h"
#include "MeshCommon.h"

CHammerSwing::CHammerSwing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CHammerSwing::CHammerSwing(const CHammerSwing& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CHammerSwing::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHammerSwing::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CHammerSwing::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CHammerSwing::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CHammerSwing::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CHammerSwing::Render()
{
	return __super::Render();
}

HRESULT CHammerSwing::Ready_EffectPartObjects()
{
	CDistortionCommon::DISTORTION_COMMON_DESC WarpDesc{};
	WarpDesc.iModelLevel = m_iPrototypeLevel;
	WarpDesc.wstrModelTag = MODEL_PROTO_TAG;
	WarpDesc.bUseTextureCom = true;
	WarpDesc.iTextureLevel = Texture_HammerSwing_Distortion.iLevelID;
	WarpDesc.wstrTextureTag = Texture_HammerSwing_Distortion.szProtoTag;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CDistortionCommon::PROTOTYPE_TAG,
		WARP_PART_TAG,
		&WarpDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC ColorDesc{};
	ColorDesc.iModelLevel = m_iPrototypeLevel;
	ColorDesc.wstrModelTag = MODEL_PROTO_TAG;
	ColorDesc.bUseTextureCom = true;
	ColorDesc.iTextureLevel = Texture_HammerSwing_Shape.iLevelID;
	ColorDesc.wstrTextureTag = Texture_HammerSwing_Shape.szProtoTag;
	ColorDesc.bUseMaskCom = true;
	ColorDesc.iMaskLevel = Texture_HammerSwing_Edge.iLevelID;
	ColorDesc.wstrMaskTag = Texture_HammerSwing_Edge.szProtoTag;
	ColorDesc.bUseUnknownTexture = true;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG,
		COLOR_PART_TAG,
		&ColorDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC YellowDesc = ColorDesc;
	YellowDesc.iTextureLevel = Texture_HammerSwing_Edge.iLevelID;
	YellowDesc.wstrTextureTag = Texture_HammerSwing_Edge.szProtoTag;
	YellowDesc.iMaskLevel = Texture_HammerSwing_Shape.iLevelID;
	YellowDesc.wstrMaskTag = Texture_HammerSwing_Shape.szProtoTag;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG,
		YELLOW_PART_TAG,
		&YellowDesc)))
		return E_FAIL;

	return S_OK;
}

CHammerSwing* CHammerSwing::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CHammerSwing* pInstance = new CHammerSwing(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CHammerSwing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHammerSwing::Clone(void* pArg)
{
	CHammerSwing* pInstance = new CHammerSwing(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CHammerSwing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHammerSwing::Free()
{
	__super::Free();
}
