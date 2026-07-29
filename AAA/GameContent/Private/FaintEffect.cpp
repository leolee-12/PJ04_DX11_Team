#include "FaintEffect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshEmitterCommon.h"

CFaintEffect::CFaintEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CFaintEffect::CFaintEffect(const CFaintEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CFaintEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFaintEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CFaintEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CFaintEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CFaintEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CFaintEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CFaintEffect::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tMesh{};

	// StarSmooth Model Part 00 ~ 02 

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};

	// SmokeLowPoly Model Part 00 ~ 02

	return S_OK;
}

CFaintEffect* CFaintEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFaintEffect* pInstance = new CFaintEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFaintEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFaintEffect::Clone(void* pArg)
{
	CFaintEffect* pInstance = new CFaintEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CFaintEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFaintEffect::Free()
{
	__super::Free();
}
