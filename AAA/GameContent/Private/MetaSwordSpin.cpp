#include "MetaSwordSpin.h"

#include "MeshCommon.h"

CMetaSwordSpin::CMetaSwordSpin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMetaSwordSpin::CMetaSwordSpin(const CMetaSwordSpin& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CMetaSwordSpin::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMetaSwordSpin::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMetaSwordSpin::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMetaSwordSpin::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMetaSwordSpin::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMetaSwordSpin::Render()
{
	return __super::Render();
}

HRESULT CMetaSwordSpin::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC Desc{};
	Desc.iModelLevel = m_iPrototypeLevel;
	Desc.wstrModelTag = MODEL_PROTO_TAG;
	Desc.bUseTextureCom = true;
	Desc.iTextureLevel = m_iPrototypeLevel;
	Desc.wstrTextureTag = TAIL_TEXTURE_PROTO_TAG;
	Desc.bUseMaskCom = true;
	Desc.iMaskLevel = m_iPrototypeLevel;
	Desc.wstrMaskTag = SCROLL_TEXTURE_PROTO_TAG;
	Desc.bUseUnknownTexture = true;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG,
		OUTER_PART_TAG,
		&Desc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG,
		CORE_PART_TAG,
		&Desc)))
		return E_FAIL;

	return S_OK;
}

CMetaSwordSpin* CMetaSwordSpin::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CMetaSwordSpin* pInstance = new CMetaSwordSpin(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMetaSwordSpin");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMetaSwordSpin::Clone(void* pArg)
{
	CMetaSwordSpin* pInstance = new CMetaSwordSpin(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMetaSwordSpin");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMetaSwordSpin::Free()
{
	__super::Free();
}
