#include "Assault_Smoke.h"
#include "GameContent_const.h"

#include "MeshParticleCommon.h"

CAssault_Smoke::CAssault_Smoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CAssault_Smoke::CAssault_Smoke(const CAssault_Smoke& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CAssault_Smoke::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAssault_Smoke::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CAssault_Smoke::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CAssault_Smoke::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CAssault_Smoke::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CAssault_Smoke::Render()
{
	return __super::Render();
}

HRESULT CAssault_Smoke::Ready_EffectPartObjects()
{
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"SmokeR", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"SmokeL", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CAssault_Smoke* CAssault_Smoke::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAssault_Smoke* pInstance = new CAssault_Smoke(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CAssault_Smoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAssault_Smoke::Clone(void* pArg)
{
	CAssault_Smoke* pInstance = new CAssault_Smoke(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CAssault_Smoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAssault_Smoke::Free()
{
	__super::Free();
}
