#include "QuadCommon.h"
#include "GameContent_const.h"

#include "GameInstance.h"

CQuadCommon::CQuadCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Quad{ pDevice, pContext }
	, m_iRenderGroup{ static_cast<_int>(RENDERID::BLEND) }
{
}

CQuadCommon::CQuadCommon(const CQuadCommon& Prototype)
	: CEffect_Quad(Prototype)
	, m_iRenderGroup{ Prototype.m_iRenderGroup }
{
}

HRESULT CQuadCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iRenderGroup = static_cast<_int>(RENDERID::BLEND);
	return S_OK;
}

HRESULT CQuadCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	QUAD_COMMON_DESC tDesc = *static_cast<QUAD_COMMON_DESC*>(pArg);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CQuadCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CQuadCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CQuadCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false)
		return;

	Compute_CombinedWorldMatrix();
	Helper::IntClamp(m_iRenderGroup, static_cast<_int>(RENDERID::PRIORITY), static_cast<_int>(RENDERID::END) - 1);
	m_pGameInstance_Proxy->Add_RenderGroup(static_cast<RENDERID>(m_iRenderGroup), this);
}

HRESULT CQuadCommon::Render()
{
	return __super::Render();
}

CQuadCommon* CQuadCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CQuadCommon* pInstance = new CQuadCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CQuadCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CQuadCommon::Clone(void* pArg)
{
	CQuadCommon* pInstance = new CQuadCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CQuadCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQuadCommon::Free()
{
	__super::Free();
}
