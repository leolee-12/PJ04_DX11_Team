#include "CarLanding.h"

#include "MeshParticleCommon.h"

CCarLanding::CCarLanding(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CCarLanding::CCarLanding(const CCarLanding& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CCarLanding::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCarLanding::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CCarLanding::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CCarLanding::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CCarLanding::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CCarLanding::Render()
{
	return __super::Render();
}

HRESULT CCarLanding::Ready_EffectPartObjects()
{
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;
	tDesc.bUseNormalTexture = false;
	tDesc.bUseMRATexture = false;
	tDesc.bUseTextureCom = false;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeSphereOriginal";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG,
		L"SmokeSphereOriginal", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeLowPoly";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG,
		L"SmokeLowPoly", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CCarLanding* CCarLanding::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCarLanding* pInstance = new CCarLanding(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CCarLanding");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCarLanding::Clone(void* pArg)
{
	CCarLanding* pInstance = new CCarLanding(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCarLanding");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCarLanding::Free()
{
	__super::Free();
}
