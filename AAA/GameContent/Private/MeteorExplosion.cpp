#include "MeteorExplosion.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "MeshParticleCommon.h"

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
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tMeshParticle{};
	tMeshParticle.iModelLevel = m_iPrototypeLevel;
	tMeshParticle.bUseDiffuseTexture = true;
	tMeshParticle.bUseNormalTexture = true;
	tMeshParticle.bUseMRATexture = true;
	tMeshParticle.bUseUnknownTexture = false;
	tMeshParticle.bUseTextureCom = false;
	tMeshParticle.bUseMaskCom = false;
	tMeshParticle.bCustomShader = false;
	
	// MeshParticleCommon - MeteorPieceSmall
	tMeshParticle.wstrModelTag = PIECE_SMALL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"PieceSmall", &tMeshParticle)))
		return E_FAIL;

	// MeshParticleCommon - MeteorPieceCool
	tMeshParticle.wstrModelTag = PIECE_COOL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"PieceCool", &tMeshParticle)))
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
