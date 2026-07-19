#include "LandingSmoke.h"

#include "MeshCommon.h"

CLandingSmoke::CLandingSmoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLandingSmoke::CLandingSmoke(const CLandingSmoke& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLandingSmoke::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLandingSmoke::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLandingSmoke::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLandingSmoke::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLandingSmoke::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLandingSmoke::Render()
{
	return __super::Render();
}

HRESULT CLandingSmoke::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tSphereDesc{};
	tSphereDesc.iModelLevel = m_iPrototypeLevel;
	tSphereDesc.wstrModelTag = L"Prototype_Component_Model_SmokeMesh";
	tSphereDesc.bUseDiffuseTexture = false;
	tSphereDesc.bUseUnknownTexture = true;
	tSphereDesc.bUseTextureCom = false;
	tSphereDesc.bUseMaskCom = false;
	tSphereDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG,
		L"SmokeSphereOriginal", &tSphereDesc)))
		return E_FAIL;

	CMeshCommon::MESH_COMMON_DESC tLowPolyDesc{};
	tLowPolyDesc.iModelLevel = m_iPrototypeLevel;
	tLowPolyDesc.wstrModelTag = L"Prototype_Component_Model_SmokeLowPolyMesh";
	tLowPolyDesc.bUseDiffuseTexture = false;
	tLowPolyDesc.bUseUnknownTexture = true;
	tLowPolyDesc.bUseTextureCom = false;
	tLowPolyDesc.bUseMaskCom = false;
	tLowPolyDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG,
		L"Prototype_Component_Model_SmokeLowPoly_1", &tLowPolyDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG,
		L"Prototype_Component_Model_SmokeLowPoly_2", &tLowPolyDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG,
		L"Prototype_Component_Model_SmokeLowPoly_3", &tLowPolyDesc)))
		return E_FAIL;

	return S_OK;
}

CLandingSmoke* CLandingSmoke::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CLandingSmoke* pInstance = new CLandingSmoke(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLandingSmoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLandingSmoke::Clone(void* pArg)
{
	CLandingSmoke* pInstance = new CLandingSmoke(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLandingSmoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLandingSmoke::Free()
{
	__super::Free();
}
