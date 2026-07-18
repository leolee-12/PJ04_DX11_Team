#include "Nail_Smoke.h"
#include "GameContent_const.h"

#include "MeshParticleCommon.h"

CNail_Smoke::CNail_Smoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CNail_Smoke::CNail_Smoke(const CNail_Smoke& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CNail_Smoke::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNail_Smoke::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CNail_Smoke::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CNail_Smoke::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CNail_Smoke::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CNail_Smoke::Render()
{
	return __super::Render();
}

HRESULT CNail_Smoke::Ready_EffectPartObjects()
{
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"Smoke", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CNail_Smoke* CNail_Smoke::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNail_Smoke* pInstance = new CNail_Smoke(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CNail_Smoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNail_Smoke::Clone(void* pArg)
{
	CNail_Smoke* pInstance = new CNail_Smoke(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CNail_Smoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNail_Smoke::Free()
{
	__super::Free();
}
