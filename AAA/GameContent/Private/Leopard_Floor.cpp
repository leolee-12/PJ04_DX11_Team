#include "Leopard_Floor.h"
#include "GameContent_const.h"

#include "MeshParticleCommon.h"

CLeopard_Floor::CLeopard_Floor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_Floor::CLeopard_Floor(const CLeopard_Floor& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_Floor::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_Floor::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_Floor::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_Floor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_Floor::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_Floor::Render()
{
	return __super::Render();
}

HRESULT CLeopard_Floor::Ready_EffectPartObjects()
{
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseDiffuseTexture = true;
	tDesc.bUseNormalTexture = true;
	tDesc.bUseMRATexture = true;

	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"Floor", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"FloorS", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_Floor* CLeopard_Floor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_Floor* pInstance = new CLeopard_Floor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_Floor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_Floor::Clone(void* pArg)
{
	CLeopard_Floor* pInstance = new CLeopard_Floor(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_Floor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_Floor::Free()
{
	__super::Free();
}
