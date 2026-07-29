#include "HammerFire.h"

#include "GameContent_const.h"
#include "DistortionCommon.h"
#include "MeshCommon.h"

CHammerFire::CHammerFire(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CHammerFire::CHammerFire(const CHammerFire& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CHammerFire::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHammerFire::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CHammerFire::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CHammerFire::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CHammerFire::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CHammerFire::Render()
{
	return __super::Render();
}

HRESULT CHammerFire::Ready_EffectPartObjects()
{
	CDistortionCommon::DISTORTION_COMMON_DESC WarpDesc{};
	WarpDesc.iModelLevel = m_iPrototypeLevel;
	WarpDesc.wstrModelTag = MODEL_PROTO_TAG_FIRE01;
	WarpDesc.bUseTextureCom = true;
	WarpDesc.iTextureLevel = Texture_HammerFire_Distortion.iLevelID;
	WarpDesc.wstrTextureTag = Texture_HammerFire_Distortion.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CDistortionCommon::PROTOTYPE_TAG, WARP_PART_TAG, &WarpDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC OuterDesc{};
	OuterDesc.iModelLevel = m_iPrototypeLevel;
	OuterDesc.wstrModelTag = MODEL_PROTO_TAG_FIRE01;
	OuterDesc.bUseTextureCom = true;
	OuterDesc.iTextureLevel = Texture_HammerFire_Scroll.iLevelID;
	OuterDesc.wstrTextureTag = Texture_HammerFire_Scroll.szProtoTag;
	OuterDesc.bUseMaskCom = true;
	OuterDesc.iMaskLevel = Texture_HammerFire_Mask.iLevelID;
	OuterDesc.wstrMaskTag = Texture_HammerFire_Mask.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, OUTER_PART_TAG, &OuterDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC CoreDesc = OuterDesc;
	CoreDesc.wstrModelTag = MODEL_PROTO_TAG_FIRE03;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, CORE_PART_TAG, &CoreDesc)))
		return E_FAIL;

	return S_OK;
}

CHammerFire* CHammerFire::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHammerFire* pInstance = new CHammerFire(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CHammerFire");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHammerFire::Clone(void* pArg)
{
	CHammerFire* pInstance = new CHammerFire(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CHammerFire");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHammerFire::Free()
{
	__super::Free();
}
