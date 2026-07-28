#include "Meta_Rock.h"
#include "GameContent_const.h"

#include "MeshParticleCommon.h"

CMeta_Rock::CMeta_Rock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_Rock::CMeta_Rock(const CMeta_Rock& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_Rock::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_Rock::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_Rock::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_Rock::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_Rock::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_Rock::Render()
{
	return __super::Render();
}

HRESULT CMeta_Rock::Ready_EffectPartObjects()
{
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = false;

	tDesc.bUseDiffuseTexture = true;
	tDesc.bUseNormalTexture = true;
	tDesc.bUseMRATexture = true;

	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_ROCK;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"Rock", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMeta_Rock* CMeta_Rock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_Rock* pInstance = new CMeta_Rock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_Rock");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_Rock::Clone(void* pArg)
{
	CMeta_Rock* pInstance = new CMeta_Rock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_Rock");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_Rock::Free()
{
	__super::Free();
}
