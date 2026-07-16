#include "SmokeCollection.h"

#include "MeshCommon.h"
#include "SmokeLowPoly.h"

CSmokeCollection::CSmokeCollection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSmokeCollection::CSmokeCollection(const CSmokeCollection& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CSmokeCollection::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSmokeCollection::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSmokeCollection::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSmokeCollection::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSmokeCollection::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSmokeCollection::Render()
{
	return __super::Render();
}

HRESULT CSmokeCollection::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeSphereOriginal";
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;
	tDesc.bUseTextureCom = false;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG,
		L"SmokeSphereOriginal", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeLowPoly::PROTOTYPE_TAG,
		L"Prototype_Component_Model_SmokeLowPoly_1")))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeLowPoly::PROTOTYPE_TAG,
		L"Prototype_Component_Model_SmokeLowPoly_2")))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeLowPoly::PROTOTYPE_TAG,
		L"Prototype_Component_Model_SmokeLowPoly_3")))
		return E_FAIL;

	return S_OK;
}

CSmokeCollection* CSmokeCollection::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CSmokeCollection* pInstance = new CSmokeCollection(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSmokeCollection");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSmokeCollection::Clone(void* pArg)
{
	CSmokeCollection* pInstance = new CSmokeCollection(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSmokeCollection");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSmokeCollection::Free()
{
	__super::Free();
}
