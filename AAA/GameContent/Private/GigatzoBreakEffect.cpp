#include "GigatzoBreakEffect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshEmitterCommon.h"

CGigatzoBreakEffect::CGigatzoBreakEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CGigatzoBreakEffect::CGigatzoBreakEffect(const CGigatzoBreakEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CGigatzoBreakEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGigatzoBreakEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CGigatzoBreakEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGigatzoBreakEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CGigatzoBreakEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CGigatzoBreakEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGigatzoBreakEffect::Ready_EffectPartObjects()
{
	// Brief expanding flash sphere. Same construction as MeteorExplosion's
	// Blast: model diffuse slot driven through the rock effect shader.
	CMeshCommon::MESH_COMMON_DESC tMesh{};
	tMesh.iModelLevel		= m_iPrototypeLevel;
	tMesh.bUseDiffuseTexture	= true;
	tMesh.bUseNormalTexture		= false;
	tMesh.bUseMRATexture		= false;
	tMesh.bUseUnknownTexture	= false;
	tMesh.bUseTextureCom		= false;
	tMesh.bUseMaskCom		= false;
	tMesh.bCustomShader		= true;
	tMesh.iShaderLevel		= Shader_EffectRock.iLevelID;
	tMesh.wstrShaderTag		= Shader_EffectRock.szProtoTag;

	tMesh.wstrModelTag = SPHERE_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG, L"Sphere", &tMesh)))
		return E_FAIL;

	// Rock fragments. The piece models carry diffuse / normal / MRA.
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel		= m_iPrototypeLevel;
	tDesc.bUseDiffuseTexture	= true;
	tDesc.bUseNormalTexture		= true;
	tDesc.bUseMRATexture		= true;
	tDesc.bUseUnknownTexture	= false;
	tDesc.bUseTextureCom		= false;
	tDesc.bUseMaskCom		= false;
	tDesc.bCustomShader		= true;
	tDesc.iShaderLevel		= Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag		= Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = PIECE_SMALL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshEmitterCommon::PROTOTYPE_TAG, L"Piece", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = PIECE_COOL_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshEmitterCommon::PROTOTYPE_TAG, L"Stone", &tDesc)))
		return E_FAIL;

	// The dominant element: the brown smoke cloud that replaces the
	// fireball. This model only exposes UNKNOWN slots.
	tDesc.bUseDiffuseTexture	= false;
	tDesc.bUseNormalTexture		= false;
	tDesc.bUseMRATexture		= false;
	tDesc.bUseUnknownTexture	= true;

	tDesc.wstrModelTag = SMOKE_MODEL_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshEmitterCommon::PROTOTYPE_TAG, L"SmokeBig", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CGigatzoBreakEffect* CGigatzoBreakEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGigatzoBreakEffect* pInstance = new CGigatzoBreakEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGigatzoBreakEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGigatzoBreakEffect::Clone(void* pArg)
{
	CGigatzoBreakEffect* pInstance = new CGigatzoBreakEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGigatzoBreakEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGigatzoBreakEffect::Free()
{
	__super::Free();
}
