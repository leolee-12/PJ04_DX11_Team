#include "OnigorosiHammerEnd.h"

#include "GameContent_const.h"
#include "DistortionCommon.h"
#include "MeshCommon.h"

COnigorosiHammerEnd::COnigorosiHammerEnd(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

COnigorosiHammerEnd::COnigorosiHammerEnd(const COnigorosiHammerEnd& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT COnigorosiHammerEnd::Initialize_Prototype()
{
	return S_OK;
}

HRESULT COnigorosiHammerEnd::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void COnigorosiHammerEnd::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void COnigorosiHammerEnd::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void COnigorosiHammerEnd::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT COnigorosiHammerEnd::Render()
{
	return __super::Render();
}

HRESULT COnigorosiHammerEnd::Ready_EffectPartObjects()
{
	CDistortionCommon::DISTORTION_COMMON_DESC WarpDesc{};
	WarpDesc.iModelLevel = m_iPrototypeLevel;
	WarpDesc.wstrModelTag = MODEL_PROTO_TAG;
	WarpDesc.bUseTextureCom = true;
	WarpDesc.iTextureLevel = Texture_HammerSwing_Distortion.iLevelID;
	WarpDesc.wstrTextureTag = Texture_HammerSwing_Distortion.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CDistortionCommon::PROTOTYPE_TAG, WARP_PART_TAG, &WarpDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC ColorDesc{};
	ColorDesc.iModelLevel = m_iPrototypeLevel;
	ColorDesc.wstrModelTag = MODEL_PROTO_TAG;
	ColorDesc.bUseTextureCom = true;
	ColorDesc.iTextureLevel = Texture_OnigorosiHammerFirst_Fire.iLevelID;
	ColorDesc.wstrTextureTag = Texture_OnigorosiHammerFirst_Fire.szProtoTag;
	ColorDesc.bUseMaskCom = true;
	ColorDesc.iMaskLevel = Texture_HammerSwing_Edge.iLevelID;
	ColorDesc.wstrMaskTag = Texture_HammerSwing_Edge.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, COLOR_PART_TAG, &ColorDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC YellowDesc = ColorDesc;
	YellowDesc.iMaskLevel = Texture_HammerSwing_Shape.iLevelID;
	YellowDesc.wstrMaskTag = Texture_HammerSwing_Shape.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, YELLOW_PART_TAG, &YellowDesc)))
		return E_FAIL;

	return S_OK;
}

COnigorosiHammerEnd* COnigorosiHammerEnd::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COnigorosiHammerEnd* pInstance = new COnigorosiHammerEnd(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: COnigorosiHammerEnd");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* COnigorosiHammerEnd::Clone(void* pArg)
{
	COnigorosiHammerEnd* pInstance = new COnigorosiHammerEnd(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: COnigorosiHammerEnd");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void COnigorosiHammerEnd::Free()
{
	__super::Free();
}
