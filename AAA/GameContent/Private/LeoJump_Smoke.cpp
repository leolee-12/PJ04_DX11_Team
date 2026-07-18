#include "LeoJump_Smoke.h"
#include "GameContent_const.h"

#include "MeshParticleCommon.h"

CLeoJump_Smoke::CLeoJump_Smoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeoJump_Smoke::CLeoJump_Smoke(const CLeoJump_Smoke& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeoJump_Smoke::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeoJump_Smoke::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeoJump_Smoke::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeoJump_Smoke::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeoJump_Smoke::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeoJump_Smoke::Render()
{
	return __super::Render();
}

HRESULT CLeoJump_Smoke::Ready_EffectPartObjects()
{
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"Smoke", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeoJump_Smoke* CLeoJump_Smoke::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeoJump_Smoke* pInstance = new CLeoJump_Smoke(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeoJump_Smoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeoJump_Smoke::Clone(void* pArg)
{
	CLeoJump_Smoke* pInstance = new CLeoJump_Smoke(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeoJump_Smoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeoJump_Smoke::Free()
{
	__super::Free();
}
