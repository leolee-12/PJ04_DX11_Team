#include "MetaSlash2.h"

#include "MeshCommon.h"

CMetaSlash2::CMetaSlash2(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMetaSlash2::CMetaSlash2(const CMetaSlash2& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CMetaSlash2::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMetaSlash2::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMetaSlash2::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMetaSlash2::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMetaSlash2::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMetaSlash2::Render()
{
	return __super::Render();
}

HRESULT CMetaSlash2::Ready_EffectPartObjects()
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

CMetaSlash2* CMetaSlash2::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CMetaSlash2* pInstance = new CMetaSlash2(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMetaSlash2");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMetaSlash2::Clone(void* pArg)
{
	CMetaSlash2* pInstance = new CMetaSlash2(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMetaSlash2");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMetaSlash2::Free()
{
	__super::Free();
}
