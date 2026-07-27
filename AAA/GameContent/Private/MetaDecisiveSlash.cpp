#include "MetaDecisiveSlash.h"

#include "MeshCommon.h"

CMetaDecisiveSlash::CMetaDecisiveSlash(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMetaDecisiveSlash::CMetaDecisiveSlash(
	const CMetaDecisiveSlash& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CMetaDecisiveSlash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMetaDecisiveSlash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMetaDecisiveSlash::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMetaDecisiveSlash::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMetaDecisiveSlash::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMetaDecisiveSlash::Render()
{
	return __super::Render();
}

HRESULT CMetaDecisiveSlash::Ready_EffectPartObjects()
{
	const auto AddSlashLayer = [this](
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

	if (FAILED(AddSlashLayer(DEEP_BLUE_PART_TAG, CIRCLE06_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddSlashLayer(SKY_BLUE_PART_TAG, CIRCLE05_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddSlashLayer(WHITE_SLASH_PART_TAG, CIRCLE05_TEXTURE_PROTO_TAG)))
		return E_FAIL;

	return S_OK;
}

CMetaDecisiveSlash* CMetaDecisiveSlash::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CMetaDecisiveSlash* pInstance =
		new CMetaDecisiveSlash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMetaDecisiveSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMetaDecisiveSlash::Clone(void* pArg)
{
	CMetaDecisiveSlash* pInstance =
		new CMetaDecisiveSlash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMetaDecisiveSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMetaDecisiveSlash::Free()
{
	__super::Free();
}
