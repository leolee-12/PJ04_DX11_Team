#include "MeshCommon.h"
#include "GameContent_const.h"

#include "GameInstance.h"

CMeshCommon::CMeshCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Mesh{ pDevice, pContext }
	, m_iRenderGroup{ static_cast<_int>(RENDERID::NONBLEND) }
{
}

CMeshCommon::CMeshCommon(const CMeshCommon& Prototype)
	: CEffect_Mesh(Prototype)
	, m_iRenderGroup{ Prototype.m_iRenderGroup }
{
}

HRESULT CMeshCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iRenderGroup = static_cast<_int>(RENDERID::NONBLEND);
	return S_OK;
}

HRESULT CMeshCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	MESH_COMMON_DESC tDesc = *static_cast<MESH_COMMON_DESC*>(pArg);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CMeshCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeshCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeshCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false)
		return;

	Compute_CombinedWorldMatrix();
	Helper::IntClamp(m_iRenderGroup, static_cast<_int>(RENDERID::PRIORITY), static_cast<_int>(RENDERID::END) - 1);
	m_pGameInstance_Proxy->Add_RenderGroup(static_cast<RENDERID>(m_iRenderGroup), this);
}

HRESULT CMeshCommon::Render()
{
	return __super::Render();
}

CMeshCommon* CMeshCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshCommon* pInstance = new CMeshCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeshCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeshCommon::Clone(void* pArg)
{
	CMeshCommon* pInstance = new CMeshCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeshCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshCommon::Free()
{
	__super::Free();
}
