#include "SwordSpinSlash.h"

#include "DistortionCommon.h"
#include "MeshCommon.h"

CSwordSpinSlash::CSwordSpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSwordSpinSlash::CSwordSpinSlash(const CSwordSpinSlash& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSwordSpinSlash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSwordSpinSlash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSwordSpinSlash::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSwordSpinSlash::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSwordSpinSlash::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSwordSpinSlash::Render()
{
	return __super::Render();
}

HRESULT CSwordSpinSlash::Ready_EffectPartObjects()
{
	CDistortionCommon::DISTORTION_COMMON_DESC WarpDesc{};
	WarpDesc.iModelLevel = m_iPrototypeLevel;
	WarpDesc.wstrModelTag = MODEL_PROTO_TAG_RING_HIGH;
	WarpDesc.bUseTextureCom = true;
	WarpDesc.iTextureLevel = m_iPrototypeLevel;
	WarpDesc.wstrTextureTag = DISTORTION_TEXTURE_PROTO_TAG;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CDistortionCommon::PROTOTYPE_TAG,
		WARP_PART_TAG,
		&WarpDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC SpinDesc{};
	SpinDesc.iModelLevel = m_iPrototypeLevel;
	SpinDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	SpinDesc.bUseTextureCom = true;
	SpinDesc.iTextureLevel = m_iPrototypeLevel;
	SpinDesc.wstrTextureTag = SPIN_TEXTURE_PROTO_TAG;
	SpinDesc.bUseMaskCom = true;
	SpinDesc.iMaskLevel = m_iPrototypeLevel;
	SpinDesc.wstrMaskTag = MASK_TEXTURE_PROTO_TAG;
	SpinDesc.bUseUnknownTexture = false;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG,
		SPIN_PART_TAG,
		&SpinDesc)))
		return E_FAIL;

	return S_OK;
}

CSwordSpinSlash* CSwordSpinSlash::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CSwordSpinSlash* pInstance = new CSwordSpinSlash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSwordSpinSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwordSpinSlash::Clone(void* pArg)
{
	CSwordSpinSlash* pInstance = new CSwordSpinSlash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSwordSpinSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwordSpinSlash::Free()
{
	__super::Free();
}
