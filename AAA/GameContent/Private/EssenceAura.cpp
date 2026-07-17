#include "EssenceAura.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "EssenceCrown.h"

CEssenceAura::CEssenceAura(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CEssenceAura::CEssenceAura(const CEssenceAura& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CEssenceAura::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEssenceAura::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CEssenceAura::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CEssenceAura::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CEssenceAura::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CEssenceAura::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEssenceAura::Ready_EffectPartObjects()
{
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CEssenceCrown::PROTOTYPE_TAG, L"Crown00")))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CEssenceCrown::PROTOTYPE_TAG, L"Crown01")))
		return E_FAIL;

	return S_OK;
}

CEssenceAura* CEssenceAura::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEssenceAura* pInstance = new CEssenceAura(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEssenceAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEssenceAura::Clone(void* pArg)
{
	CEssenceAura* pInstance = new CEssenceAura(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEssenceAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEssenceAura::Free()
{
	__super::Free();
}
