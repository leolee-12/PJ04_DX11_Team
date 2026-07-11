#include "BubbleAura.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "Bubble.h"
#include "StarEmitter.h"
#include "StarMesh.h"

CBubbleAura::CBubbleAura(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CBubbleAura::CBubbleAura(const CBubbleAura& Prototype)
	: CEffect_Container (Prototype)
{
}

HRESULT CBubbleAura::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBubbleAura::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CBubbleAura::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CBubbleAura::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CBubbleAura::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CBubbleAura::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CBubbleAura::Ready_EffectPartObjects()
{
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CBubble::PROTOTYPE_TAG, L"RingHalo")))
		return E_FAIL;

	//if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarMesh::PROTOTYPE_TAG, L"OrbitStar00")))
	//	return E_FAIL;

	//if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarMesh::PROTOTYPE_TAG, L"OrbitStar01")))
	//	return E_FAIL;

	//if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarEmitter::PROTOTYPE_TAG, L"StarEmitter00")))
	//	return E_FAIL;	
	//
	//if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarEmitter::PROTOTYPE_TAG, L"StarEmitter01")))
	//	return E_FAIL;


	// TODO : 주위 Sparkle 추가

	return S_OK;
}

CBubbleAura* CBubbleAura::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBubbleAura* pInstance = new CBubbleAura(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBubbleAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBubbleAura::Clone(void* pArg)
{
	CBubbleAura* pInstance = new CBubbleAura(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBubbleAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBubbleAura::Free()
{
	__super::Free();
}
