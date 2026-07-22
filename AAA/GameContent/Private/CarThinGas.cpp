#include "CarThinGas.h"

#include "MeshEmitterCommon.h"

CCarThinGas::CCarThinGas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CCarThinGas::CCarThinGas(const CCarThinGas& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CCarThinGas::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCarThinGas::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CCarThinGas::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CCarThinGas::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CCarThinGas::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CCarThinGas::Render()
{
	return __super::Render();
}

HRESULT CCarThinGas::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = false;
	tDesc.bUseNormalTexture = false;
	tDesc.bUseMRATexture = false;
	tDesc.bUseTextureCom = false;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Gas1", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Gas2", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CCarThinGas* CCarThinGas::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCarThinGas* pInstance = new CCarThinGas(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CCarThinGas");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCarThinGas::Clone(void* pArg)
{
	CCarThinGas* pInstance = new CCarThinGas(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCarThinGas");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCarThinGas::Free()
{
	__super::Free();
}
