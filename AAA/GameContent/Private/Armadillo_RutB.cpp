#include "Armadillo_RutB.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CArmadillo_RutB::CArmadillo_RutB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CArmadillo_RutB::CArmadillo_RutB(const CArmadillo_RutB& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CArmadillo_RutB::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArmadillo_RutB::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CArmadillo_RutB::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CArmadillo_RutB::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CArmadillo_RutB::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CArmadillo_RutB::Render()
{
	return __super::Render();
}

HRESULT CArmadillo_RutB::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"RutB", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CArmadillo_RutB* CArmadillo_RutB::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CArmadillo_RutB* pInstance = new CArmadillo_RutB(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CArmadillo_RutB");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CArmadillo_RutB::Clone(void* pArg)
{
	CArmadillo_RutB* pInstance = new CArmadillo_RutB(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CArmadillo_RutB");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArmadillo_RutB::Free()
{
	__super::Free();
}
