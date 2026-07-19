#include "Armadillo_RutA.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CArmadillo_RutA::CArmadillo_RutA(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CArmadillo_RutA::CArmadillo_RutA(const CArmadillo_RutA& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CArmadillo_RutA::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArmadillo_RutA::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CArmadillo_RutA::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CArmadillo_RutA::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CArmadillo_RutA::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CArmadillo_RutA::Render()
{
	return __super::Render();
}

HRESULT CArmadillo_RutA::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"RutA", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CArmadillo_RutA* CArmadillo_RutA::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CArmadillo_RutA* pInstance = new CArmadillo_RutA(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CArmadillo_RutA");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CArmadillo_RutA::Clone(void* pArg)
{
	CArmadillo_RutA* pInstance = new CArmadillo_RutA(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CArmadillo_RutA");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArmadillo_RutA::Free()
{
	__super::Free();
}
