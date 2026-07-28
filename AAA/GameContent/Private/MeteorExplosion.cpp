#include "MeteorExplosion.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshEmitterCommon.h"

CMeteorExplosion::CMeteorExplosion(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeteorExplosion::CMeteorExplosion(const CMeteorExplosion& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeteorExplosion::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeteorExplosion::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeteorExplosion::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeteorExplosion::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeteorExplosion::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeteorExplosion::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMeteorExplosion::Ready_EffectPartObjects()
{
	// Stage 1 : blast volume - one big sphere, emissive drives the bloom.
	CMeshCommon::MESH_COMMON_DESC tBlast{};
	tBlast.iModelLevel        = m_iPrototypeLevel;
	tBlast.wstrModelTag       = SPHERE_MODEL_TAG;
	tBlast.bUseDiffuseTexture = true;
	tBlast.bUseNormalTexture  = false;
	tBlast.bUseMRATexture     = false;
	tBlast.bUseUnknownTexture = false;
	tBlast.bUseTextureCom     = false;
	tBlast.bUseMaskCom        = false;
	tBlast.bCustomShader      = true;
	tBlast.iShaderLevel       = Shader_Meteor.iLevelID;
	tBlast.wstrShaderTag      = Shader_Meteor.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Blast", &tBlast)))
		return E_FAIL;

	// Stage 2 : many small spheres, dither-dissolve as their alpha falls.
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel        = m_iPrototypeLevel;
	tDesc.bUseDiffuseTexture = true;
	tDesc.bUseNormalTexture  = false;
	tDesc.bUseMRATexture     = false;
	tDesc.bUseUnknownTexture = false;
	tDesc.bUseTextureCom     = false;
	tDesc.bUseMaskCom        = false;
	tDesc.bCustomShader      = true;
	tDesc.iShaderLevel       = Shader_Meteor.iLevelID;
	tDesc.wstrShaderTag      = Shader_Meteor.szProtoTag;

	tDesc.wstrModelTag = SPHERE_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Puff", &tDesc)))
		return E_FAIL;

	// Stage 3 : rock fragments. Piece models carry diffuse / normal / MRA.
	tDesc.bUseNormalTexture = true;
	tDesc.bUseMRATexture    = true;

	tDesc.wstrModelTag = PIECE_COOL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Chunk_1", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = PIECE_SMALL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Chunk_2", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = PIECE_SMALL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Chunk_3", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = PIECE_COOL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Chunk_4", &tDesc)))
		return E_FAIL;

	// Trailing smoke. This model only exposes UNKNOWN slots.
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseNormalTexture  = false;
	tDesc.bUseMRATexture     = false;
	tDesc.bUseUnknownTexture = true;

	tDesc.wstrModelTag = SMOKE_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Smoke", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMeteorExplosion* CMeteorExplosion::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeteorExplosion* pInstance = new CMeteorExplosion(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMeteorExplosion");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeteorExplosion::Clone(void* pArg)
{
	CMeteorExplosion* pInstance = new CMeteorExplosion(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeteorExplosion");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeteorExplosion::Free()
{
	__super::Free();
}
