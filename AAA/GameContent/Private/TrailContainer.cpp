#include "TrailContainer.h"

#include "TrailCommon.h"

CTrailContainer::CTrailContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CTrailContainer::CTrailContainer(const CTrailContainer& Prototype)
	: CEffect_Container ( Prototype )
{
}

HRESULT CTrailContainer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrailContainer::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CTrailContainer::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CTrailContainer::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CTrailContainer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTrailContainer::Render()
{
	return __super::Render();
}

HRESULT CTrailContainer::Ready_EffectPartObjects()
{
	CTrailCommon::TRAIL_COMMON_DESC tTrailDesc{};
	tTrailDesc.bCustomShader	= false;
	tTrailDesc.bUseTextureCom	= false;
	tTrailDesc.bUseMaskCom		= false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,	CTrailCommon::PROTOTYPE_TAG, L"Trail", &tTrailDesc)))
		return E_FAIL;

	return S_OK;
}

CTrailContainer* CTrailContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrailContainer* pInstance = new CTrailContainer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CTrailContainer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTrailContainer::Clone(void* pArg)
{
	CTrailContainer* pInstance = new CTrailContainer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CTrailContainer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrailContainer::Free()
{
	__super::Free();
}
