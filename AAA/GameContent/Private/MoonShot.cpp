#include "MoonShot.h"

#include "MeshCommon.h"

CMoonShot::CMoonShot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMoonShot::CMoonShot(const CMoonShot& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CMoonShot::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMoonShot::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMoonShot::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMoonShot::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMoonShot::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMoonShot::Render()
{
	return __super::Render();
}

HRESULT CMoonShot::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = m_iPrototypeLevel;
	tDesc.wstrTextureTag = TEXTURE_PROTO_TAG_FIRE_FORM;
	tDesc.bUseMaskCom = true;
	tDesc.iMaskLevel = m_iPrototypeLevel;
	tDesc.wstrMaskTag = TEXTURE_PROTO_TAG_FIRE_FORM;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_MOON;
	tDesc.bUseUnknownTexture = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"MoonBlue", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"MoonSky", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"MoonWhite", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMoonShot* CMoonShot::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMoonShot* pInstance = new CMoonShot(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMoonShot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMoonShot::Clone(void* pArg)
{
	CMoonShot* pInstance = new CMoonShot(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMoonShot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMoonShot::Free()
{
	__super::Free();
}
