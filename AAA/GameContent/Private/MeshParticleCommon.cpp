#include "MeshParticleCommon.h"
#include "GameContent_const.h"

#include "GameInstance.h"

CMeshParticleCommon::CMeshParticleCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_MeshParticle{ pDevice, pContext }
	, m_iRenderGroup{ static_cast<_int>(RENDERID::NONBLEND) }
{
}

CMeshParticleCommon::CMeshParticleCommon(const CMeshParticleCommon& Prototype)
	: CEffect_MeshParticle(Prototype)
	, m_iRenderGroup{ Prototype.m_iRenderGroup }
{
}

HRESULT CMeshParticleCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iRenderGroup = static_cast<_int>(RENDERID::NONBLEND);
	return S_OK;
}

HRESULT CMeshParticleCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	MESH_PARTICLE_COMMON_DESC tDesc = *static_cast<MESH_PARTICLE_COMMON_DESC*>(pArg);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CMeshParticleCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeshParticleCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeshParticleCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false)
		return;

	Helper::IntClamp(m_iRenderGroup, static_cast<_int>(RENDERID::PRIORITY), static_cast<_int>(RENDERID::END) - 1);
	m_pGameInstance_Proxy->Add_RenderGroup(static_cast<RENDERID>(m_iRenderGroup), this);
}

HRESULT CMeshParticleCommon::Render()
{
	return __super::Render();
}

CMeshParticleCommon* CMeshParticleCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshParticleCommon* pInstance = new CMeshParticleCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeshParticleCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeshParticleCommon::Clone(void* pArg)
{
	CMeshParticleCommon* pInstance = new CMeshParticleCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeshParticleCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshParticleCommon::Free()
{
	__super::Free();
}
