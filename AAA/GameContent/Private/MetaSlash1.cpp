#include "MetaSlash1.h"

#include "MeshCommon.h"

CMetaSlash1::CMetaSlash1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMetaSlash1::CMetaSlash1(const CMetaSlash1& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CMetaSlash1::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMetaSlash1::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMetaSlash1::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMetaSlash1::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMetaSlash1::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMetaSlash1::Render()
{
	return __super::Render();
}

HRESULT CMetaSlash1::Ready_EffectPartObjects()
{
	const auto AddRingPart = [this](
		const _tchar* pPartTag,
		const _tchar* pMaskTag) -> HRESULT
		{
			CMeshCommon::MESH_COMMON_DESC Desc{};
			Desc.iModelLevel = m_iPrototypeLevel;
			Desc.wstrModelTag = MODEL_PROTO_TAG_RING;
			Desc.bUseTextureCom = true;
			Desc.iTextureLevel = m_iPrototypeLevel;
			Desc.wstrTextureTag = SPIN_TEXTURE_PROTO_TAG;
			Desc.bUseMaskCom = true;
			Desc.iMaskLevel = m_iPrototypeLevel;
			Desc.wstrMaskTag = pMaskTag;
			Desc.bUseUnknownTexture = false;

			return Add_Effect_PartObject(
				m_iPrototypeLevel,
				CMeshCommon::PROTOTYPE_TAG,
				pPartTag,
				&Desc);
		};

	if (FAILED(AddRingPart(WARP_COLOR_PART_TAG, CIRCLE06_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddRingPart(WIND_PART_TAG, CIRCLE05_TEXTURE_PROTO_TAG)))
		return E_FAIL;

	return S_OK;
}

CMetaSlash1* CMetaSlash1::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CMetaSlash1* pInstance = new CMetaSlash1(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMetaSlash1");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMetaSlash1::Clone(void* pArg)
{
	CMetaSlash1* pInstance = new CMetaSlash1(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMetaSlash1");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMetaSlash1::Free()
{
	__super::Free();
}
