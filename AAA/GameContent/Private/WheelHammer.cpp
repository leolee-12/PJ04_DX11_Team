#include "WheelHammer.h"

#include "DistortionCommon.h"
#include "GameContent_const.h"
#include "MeshCommon.h"

CWheelHammer::CWheelHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CWheelHammer::CWheelHammer(const CWheelHammer& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CWheelHammer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWheelHammer::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CWheelHammer::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CWheelHammer::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CWheelHammer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CWheelHammer::Render()
{
	return __super::Render();
}

HRESULT CWheelHammer::Ready_EffectPartObjects()
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
		WARP_PART_TAG_A,
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
		COLOR_PART_TAG_A,
		&ColorDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CDistortionCommon::PROTOTYPE_TAG,
		WARP_PART_TAG_B,
		&WarpDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG,
		COLOR_PART_TAG_B,
		&ColorDesc)))
		return E_FAIL;

	return S_OK;
}

CWheelHammer* CWheelHammer::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CWheelHammer* pInstance = new CWheelHammer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CWheelHammer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWheelHammer::Clone(void* pArg)
{
	CWheelHammer* pInstance = new CWheelHammer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CWheelHammer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWheelHammer::Free()
{
	__super::Free();
}
