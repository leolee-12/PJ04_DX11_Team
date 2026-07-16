#include "SmokeSphereOriginalEmitter.h"

#include "MeshEmitterCommon.h"

CSmokeSphereOriginalEmitter::CSmokeSphereOriginalEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSmokeSphereOriginalEmitter::CSmokeSphereOriginalEmitter(const CSmokeSphereOriginalEmitter& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSmokeSphereOriginalEmitter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSmokeSphereOriginalEmitter::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSmokeSphereOriginalEmitter::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSmokeSphereOriginalEmitter::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSmokeSphereOriginalEmitter::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSmokeSphereOriginalEmitter::Render()
{
	return __super::Render();
}

HRESULT CSmokeSphereOriginalEmitter::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeSphereOriginal";
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;
	tDesc.bUseNormalTexture = false;
	tDesc.bUseMRATexture = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG,
		L"SmokeSphereOriginalEmitter", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CSmokeSphereOriginalEmitter* CSmokeSphereOriginalEmitter::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CSmokeSphereOriginalEmitter* pInstance = new CSmokeSphereOriginalEmitter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSmokeSphereOriginalEmitter");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSmokeSphereOriginalEmitter::Clone(void* pArg)
{
	CSmokeSphereOriginalEmitter* pInstance = new CSmokeSphereOriginalEmitter(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSmokeSphereOriginalEmitter");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSmokeSphereOriginalEmitter::Free()
{
	__super::Free();
}
