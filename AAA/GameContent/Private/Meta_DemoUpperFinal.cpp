#include "Meta_DemoUpperFinal.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshParticleCommon.h"

CMeta_DemoUpperFinal::CMeta_DemoUpperFinal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_DemoUpperFinal::CMeta_DemoUpperFinal(const CMeta_DemoUpperFinal& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_DemoUpperFinal::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_DemoUpperFinal::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_DemoUpperFinal::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_DemoUpperFinal::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_DemoUpperFinal::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_DemoUpperFinal::Render()
{
	return __super::Render();
}

HRESULT CMeta_DemoUpperFinal::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = false;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring01", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Line2.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Line2.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"LinePtcl", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = MODEL_PROTO_TAG_THUNDER;
	tDesc.bUseTextureCom = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"ThunderPtcl", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = MODEL_PROTO_TAG_ROCK;
	tDesc.bUseTextureCom = false;
	tDesc.bUseDiffuseTexture = true;
	tDesc.bUseUnknownTexture = false;
	tDesc.bUseNormalTexture = true;
	tDesc.bUseMRATexture = true;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"RockPtcl", &tDesc)))
		return E_FAIL;


	return S_OK;
}

CMeta_DemoUpperFinal* CMeta_DemoUpperFinal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_DemoUpperFinal* pInstance = new CMeta_DemoUpperFinal(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_DemoUpperFinal");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_DemoUpperFinal::Clone(void* pArg)
{
	CMeta_DemoUpperFinal* pInstance = new CMeta_DemoUpperFinal(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_DemoUpperFinal");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_DemoUpperFinal::Free()
{
	__super::Free();
}
