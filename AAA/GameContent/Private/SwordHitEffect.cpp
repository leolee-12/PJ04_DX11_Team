#include "SwordHitEffect.h"

#include "MeshEmitterCommon.h"

CSwordHitEffect::CSwordHitEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSwordHitEffect::CSwordHitEffect(const CSwordHitEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSwordHitEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSwordHitEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSwordHitEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSwordHitEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSwordHitEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSwordHitEffect::Render()
{
	return __super::Render();
}

HRESULT CSwordHitEffect::Ready_EffectPartObjects()
{

	return S_OK;
}

CSwordHitEffect* CSwordHitEffect::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CSwordHitEffect* pInstance = new CSwordHitEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSwordHitEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwordHitEffect::Clone(void* pArg)
{
	CSwordHitEffect* pInstance = new CSwordHitEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSwordHitEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwordHitEffect::Free()
{
	__super::Free();
}
