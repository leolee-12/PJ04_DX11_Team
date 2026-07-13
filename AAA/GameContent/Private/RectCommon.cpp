#include "RectCommon.h"
#include "GameContent_const.h"

#include "GameInstance.h"

CRectCommon::CRectCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Quad{ pDevice, pContext }
	, m_iRenderGroup{ static_cast<_int>(RENDERID::BLEND) }
{
}

CRectCommon::CRectCommon(const CRectCommon& Prototype)
	: CEffect_Quad(Prototype)
	, m_iRenderGroup{ Prototype.m_iRenderGroup }
{
}

HRESULT CRectCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iRenderGroup = static_cast<_int>(RENDERID::BLEND);
	return S_OK;
}

HRESULT CRectCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	RECT_COMMON_DESC tDesc = *static_cast<RECT_COMMON_DESC*>(pArg);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CRectCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CRectCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CRectCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false)
		return;

	Compute_CombinedWorldMatrix();
	Helper::IntClamp(m_iRenderGroup, static_cast<_int>(RENDERID::PRIORITY), static_cast<_int>(RENDERID::END) - 1);
	m_pGameInstance_Proxy->Add_RenderGroup(static_cast<RENDERID>(m_iRenderGroup), this);
}

HRESULT CRectCommon::Render()
{
	return __super::Render();
}

CRectCommon* CRectCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRectCommon* pInstance = new CRectCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CRectCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRectCommon::Clone(void* pArg)
{
	CRectCommon* pInstance = new CRectCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CRectCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRectCommon::Free()
{
	__super::Free();
}
