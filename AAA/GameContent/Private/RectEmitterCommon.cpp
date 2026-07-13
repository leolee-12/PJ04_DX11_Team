#include "RectEmitterCommon.h"
#include "GameContent_const.h"

#include "GameInstance.h"

CRectEmitterCommon::CRectEmitterCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_RectEmitter{ pDevice, pContext }
	, m_iRenderGroup{ static_cast<_int>(RENDERID::BLEND) }
{
}

CRectEmitterCommon::CRectEmitterCommon(const CRectEmitterCommon& Prototype)
	: CEffect_RectEmitter(Prototype)
	, m_iRenderGroup{ Prototype.m_iRenderGroup }
{
}

HRESULT CRectEmitterCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iRenderGroup = static_cast<_int>(RENDERID::BLEND);
	return S_OK;
}

HRESULT CRectEmitterCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	RECT_EMITTER_COMMON_DESC tDesc = *static_cast<RECT_EMITTER_COMMON_DESC*>(pArg);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CRectEmitterCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CRectEmitterCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CRectEmitterCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false)
		return;

	Helper::IntClamp(m_iRenderGroup, static_cast<_int>(RENDERID::PRIORITY), static_cast<_int>(RENDERID::END) - 1);
	m_pGameInstance_Proxy->Add_RenderGroup(static_cast<RENDERID>(m_iRenderGroup), this);
}

HRESULT CRectEmitterCommon::Render()
{
	Set_UseGBufferOutput(static_cast<RENDERID>(m_iRenderGroup) == RENDERID::NONBLEND);
	return __super::Render();
}

CRectEmitterCommon* CRectEmitterCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRectEmitterCommon* pInstance = new CRectEmitterCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CRectEmitterCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRectEmitterCommon::Clone(void* pArg)
{
	CRectEmitterCommon* pInstance = new CRectEmitterCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CRectEmitterCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRectEmitterCommon::Free()
{
	__super::Free();
}
