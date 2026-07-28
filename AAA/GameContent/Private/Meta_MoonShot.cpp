#include "Meta_MoonShot.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CMeta_MoonShot::CMeta_MoonShot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_MoonShot::CMeta_MoonShot(const CMeta_MoonShot& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_MoonShot::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_MoonShot::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_MoonShot::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_MoonShot::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_MoonShot::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_MoonShot::Render()
{
	return __super::Render();
}

HRESULT CMeta_MoonShot::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = false;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Wave", &tDesc)))
		return E_FAIL;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_TOP;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"WaveTop", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMeta_MoonShot* CMeta_MoonShot::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_MoonShot* pInstance = new CMeta_MoonShot(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_MoonShot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_MoonShot::Clone(void* pArg)
{
	CMeta_MoonShot* pInstance = new CMeta_MoonShot(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_MoonShot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_MoonShot::Free()
{
	__super::Free();
}
