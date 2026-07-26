#include "SwordJumpSpin.h"

#include "MeshCommon.h"

CSwordJumpSpin::CSwordJumpSpin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSwordJumpSpin::CSwordJumpSpin(const CSwordJumpSpin& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CSwordJumpSpin::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSwordJumpSpin::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSwordJumpSpin::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSwordJumpSpin::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSwordJumpSpin::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSwordJumpSpin::Render()
{
	return __super::Render();
}

HRESULT CSwordJumpSpin::Ready_EffectPartObjects()
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

CSwordJumpSpin* CSwordJumpSpin::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CSwordJumpSpin* pInstance = new CSwordJumpSpin(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSwordJumpSpin");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwordJumpSpin::Clone(void* pArg)
{
	CSwordJumpSpin* pInstance = new CSwordJumpSpin(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSwordJumpSpin");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwordJumpSpin::Free()
{
	__super::Free();
}
