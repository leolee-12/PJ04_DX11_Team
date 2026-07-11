#include "RectParticleCommon.h"
#include "GameContent_const.h"

#include "GameInstance.h"

CRectParticleCommon::CRectParticleCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_RectParticle{ pDevice, pContext }
	, m_iRenderGroup{ static_cast<_int>(RENDERID::BLEND) }
{
}

CRectParticleCommon::CRectParticleCommon(const CRectParticleCommon& Prototype)
	: CEffect_RectParticle(Prototype)
	, m_iRenderGroup{ Prototype.m_iRenderGroup }
{
}

HRESULT CRectParticleCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iRenderGroup = static_cast<_int>(RENDERID::BLEND);
	return S_OK;
}

HRESULT CRectParticleCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	RECT_PARTICLE_COMMON_DESC tDesc = *static_cast<RECT_PARTICLE_COMMON_DESC*>(pArg);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CRectParticleCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CRectParticleCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CRectParticleCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false)
		return;

	Helper::IntClamp(m_iRenderGroup, static_cast<_int>(RENDERID::PRIORITY), static_cast<_int>(RENDERID::END) - 1);
	m_pGameInstance_Proxy->Add_RenderGroup(static_cast<RENDERID>(m_iRenderGroup), this);
}

HRESULT CRectParticleCommon::Render()
{
	return __super::Render();
}

CRectParticleCommon* CRectParticleCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRectParticleCommon* pInstance = new CRectParticleCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CRectParticleCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRectParticleCommon::Clone(void* pArg)
{
	CRectParticleCommon* pInstance = new CRectParticleCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CRectParticleCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRectParticleCommon::Free()
{
	__super::Free();
}
