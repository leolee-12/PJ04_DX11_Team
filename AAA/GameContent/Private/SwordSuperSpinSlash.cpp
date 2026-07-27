#include "SwordSuperSpinSlash.h"

#include "MeshCommon.h"

CSwordSuperSpinSlash::CSwordSuperSpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSwordSuperSpinSlash::CSwordSuperSpinSlash(const CSwordSuperSpinSlash& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CSwordSuperSpinSlash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSwordSuperSpinSlash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSwordSuperSpinSlash::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSwordSuperSpinSlash::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSwordSuperSpinSlash::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSwordSuperSpinSlash::Render()
{
	return __super::Render();
}

HRESULT CSwordSuperSpinSlash::Ready_EffectPartObjects()
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

	if (FAILED(AddRingPart(OUTLINE_PART_TAG, SPIN01_TEXTURE_PROTO_TAG, CIRCLE04_TEXTURE_PROTO_TAG)))
		return E_FAIL;
	if (FAILED(AddRingPart(HUKAI_PART_TAG, SPIN06_TEXTURE_PROTO_TAG, nullptr)))
		return E_FAIL;
	if (FAILED(AddRingPart(WHITE_LINE_PART_TAG, SPIN06_TEXTURE_PROTO_TAG, nullptr)))
		return E_FAIL;

	return S_OK;
}

CSwordSuperSpinSlash* CSwordSuperSpinSlash::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CSwordSuperSpinSlash* pInstance = new CSwordSuperSpinSlash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSwordSuperSpinSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwordSuperSpinSlash::Clone(void* pArg)
{
	CSwordSuperSpinSlash* pInstance = new CSwordSuperSpinSlash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSwordSuperSpinSlash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwordSuperSpinSlash::Free()
{
	__super::Free();
}
