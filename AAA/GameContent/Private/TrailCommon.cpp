#include "TrailCommon.h"

#include "GameContent_const.h"
#include "GameInstance.h"

CTrailCommon::CTrailCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Trail{ pDevice, pContext }
	, m_iRenderGroup{ static_cast<_int>(RENDERID::BLEND) }
{
}

CTrailCommon::CTrailCommon(const CTrailCommon& Prototype)
	: CEffect_Trail{ Prototype }
	, m_iRenderGroup{ Prototype.m_iRenderGroup }
{
}

HRESULT CTrailCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iRenderGroup = static_cast<_int>(RENDERID::BLEND);
	return S_OK;
}

HRESULT CTrailCommon::Initialize(void* pArg)
{
	if (pArg == nullptr)
		return E_FAIL;

    TRAIL_COMMON_DESC TrailDesc = *static_cast<TRAIL_COMMON_DESC*>(pArg);

    if (TrailDesc.wstrVIBufferTag.empty())
    {
        TrailDesc.iVIBufferLevel = VI_Trail.iLevelID;
        TrailDesc.wstrVIBufferTag = VI_Trail.szProtoTag;
    }

    return __super::Initialize(&TrailDesc);
}

void CTrailCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CTrailCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CTrailCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (Is_TrailRenderable() == false)
		return;

	Helper::IntClamp(m_iRenderGroup,
		static_cast<_int>(RENDERID::PRIORITY),
		static_cast<_int>(RENDERID::END) - 1);
	m_pGameInstance_Proxy->Add_RenderGroup(static_cast<RENDERID>(m_iRenderGroup), this);
}

HRESULT CTrailCommon::Render()
{
	return __super::Render();
}

CTrailCommon* CTrailCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrailCommon* pInstance = new CTrailCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CTrailCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTrailCommon::Clone(void* pArg)
{
	CTrailCommon* pInstance = new CTrailCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CTrailCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrailCommon::Free()
{
	__super::Free();
}
