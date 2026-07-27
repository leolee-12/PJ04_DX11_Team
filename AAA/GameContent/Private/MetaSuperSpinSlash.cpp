#include "MetaSuperSpinSlash.h"

#include "GameContent_const.h"
#include "MeshCommon.h"
#include "RectParticleCommon.h"

CMetaSuperSpinSlash::CMetaSuperSpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMetaSuperSpinSlash::CMetaSuperSpinSlash(const CMetaSuperSpinSlash& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CMetaSuperSpinSlash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMetaSuperSpinSlash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMetaSuperSpinSlash::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMetaSuperSpinSlash::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMetaSuperSpinSlash::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMetaSuperSpinSlash::Render()
{
	return __super::Render();
}

HRESULT CMetaSuperSpinSlash::Ready_EffectPartObjects()
{
	const auto AddRingPart = [this](
		const _tchar* pPartTag,
		const _tchar* pTextureTag,
		const _tchar* pMaskTag) -> HRESULT
		{
			CMeshCommon::MESH_COMMON_DESC Desc{};
			Desc.iModelLevel = m_iPrototypeLevel;
			Desc.wstrModelTag = MODEL_PROTO_TAG_RING;
			Desc.bUseTextureCom = true;
			Desc.iTextureLevel = m_iPrototypeLevel;
			Desc.wstrTextureTag = pTextureTag;
			Desc.bUseMaskCom = pMaskTag != nullptr;
			Desc.iMaskLevel = m_iPrototypeLevel;
			if (pMaskTag != nullptr)
				Desc.wstrMaskTag = pMaskTag;
			Desc.bUseUnknownTexture = false;

			return Add_Effect_PartObject(
				m_iPrototypeLevel,
				CMeshCommon::PROTOTYPE_TAG,
				pPartTag,
				&Desc);
		};

	if (FAILED(AddRingPart(OUTLINE_DARK_PART_TAG, SPIN01_TEXTURE_PROTO_TAG, CIRCLE04_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddRingPart(OUTLINE_COPY_PART_TAG, SPIN01_TEXTURE_PROTO_TAG, CIRCLE04_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddRingPart(HUKAI_COPY_PART_TAG, SPIN06_TEXTURE_PROTO_TAG, nullptr)))
		return E_FAIL;
	if (FAILED(AddRingPart(OUTLINE_PART_TAG, SPIN01_TEXTURE_PROTO_TAG, CIRCLE04_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddRingPart(HUKAI_PART_TAG, SPIN06_TEXTURE_PROTO_TAG, nullptr)))
		return E_FAIL;
	if (FAILED(AddRingPart(SPIN3_PART_TAG, SPIN06_TEXTURE_PROTO_TAG, CIRCLE05_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddRingPart(WHITE_LINE_PART_TAG, SPIN06_TEXTURE_PROTO_TAG, nullptr)))
		return E_FAIL;

	CRectParticleCommon::RECT_PARTICLE_COMMON_DESC PowderDesc{};
	PowderDesc.iVIBufferLevel = VI_Rect.iLevelID;
	PowderDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	PowderDesc.bUseTextureCom = true;
	PowderDesc.iTextureLevel = m_iPrototypeLevel;
	PowderDesc.wstrTextureTag = CIRCLE01_TEXTURE_PROTO_TAG;
	PowderDesc.bUseMaskCom = false;
	PowderDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CRectParticleCommon::PROTOTYPE_TAG,
		SHINE_POWDER_PART_TAG,
		&PowderDesc)))
		return E_FAIL;

	return S_OK;
}

CMetaSuperSpinSlash* CMetaSuperSpinSlash::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CMetaSuperSpinSlash* pInstance = new CMetaSuperSpinSlash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMetaSuperSpinSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMetaSuperSpinSlash::Clone(void* pArg)
{
	CMetaSuperSpinSlash* pInstance = new CMetaSuperSpinSlash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMetaSuperSpinSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMetaSuperSpinSlash::Free()
{
	__super::Free();
}
