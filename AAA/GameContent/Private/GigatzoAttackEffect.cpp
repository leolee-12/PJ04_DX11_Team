#include "GigatzoAttackEffect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "MeshEmitterCommon.h"
#include "RectEmitterCommon.h"
#include "RectCommon.h"

CGigatzoAttackEffect::CGigatzoAttackEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CGigatzoAttackEffect::CGigatzoAttackEffect(const CGigatzoAttackEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CGigatzoAttackEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGigatzoAttackEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CGigatzoAttackEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGigatzoAttackEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CGigatzoAttackEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CGigatzoAttackEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGigatzoAttackEffect::Ready_EffectPartObjects()
{
	// Muzzle smoke puffs. The mesh exposes an UNKNOWN slot only
	// (white dummy Sample.png), so colour comes from part properties.
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tSmoke{};
	tSmoke.iModelLevel			= m_iPrototypeLevel;
	tSmoke.bUseDiffuseTexture	= false;
	tSmoke.bUseUnknownTexture	= true;
	tSmoke.bUseNormalTexture	= false;
	tSmoke.bUseMRATexture		= false;
	tSmoke.bUseTextureCom		= false;
	tSmoke.bUseMaskCom			= false;
	tSmoke.bCustomShader		= false;
	tSmoke.iShaderLevel			= 0;
	tSmoke.wstrShaderTag		= L"";
	tSmoke.wstrModelTag			= SMOKE_MODEL_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshEmitterCommon::PROTOTYPE_TAG, L"Smoke", &tSmoke)))
		return E_FAIL;

	// Sparks / flame licks / warm smoke. One texture per part.
	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tEmitter{};
	tEmitter.iVIBufferLevel		= VI_Rect.iLevelID;
	tEmitter.wstrVIBufferTag	= VI_Rect.szProtoTag;
	tEmitter.bUseTextureCom		= true;
	tEmitter.bUseMaskCom		= false;
	tEmitter.bCustomShader		= false;

	tEmitter.iTextureLevel		= Texture_GigatzoSpark.iLevelID;
	tEmitter.wstrTextureTag		= Texture_GigatzoSpark.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CRectEmitterCommon::PROTOTYPE_TAG, L"Spark", &tEmitter)))
		return E_FAIL;

	tEmitter.iTextureLevel		= Texture_GigatzoFire.iLevelID;
	tEmitter.wstrTextureTag		= Texture_GigatzoFire.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CRectEmitterCommon::PROTOTYPE_TAG, L"Fire", &tEmitter)))
		return E_FAIL;

	tEmitter.iTextureLevel		= Texture_GigatzoFireSmoke.iLevelID;
	tEmitter.wstrTextureTag		= Texture_GigatzoFireSmoke.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CRectEmitterCommon::PROTOTYPE_TAG, L"FireSmoke", &tEmitter)))
		return E_FAIL;

	// Bloom-only stand-in for the original PointLight emitter.
	// It does not illuminate geometry.
	CRectCommon::RECT_COMMON_DESC tGlow{};
	tGlow.iVIBufferLevel	= VI_Rect.iLevelID;
	tGlow.wstrVIBufferTag	= VI_Rect.szProtoTag;
	tGlow.bUseTextureCom	= true;
	tGlow.iTextureLevel		= Texture_GigatzoFireSmoke.iLevelID;
	tGlow.wstrTextureTag	= Texture_GigatzoFireSmoke.szProtoTag;
	tGlow.bUseMaskCom		= false;
	tGlow.bCustomShader		= false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CRectCommon::PROTOTYPE_TAG, L"Glow", &tGlow)))
		return E_FAIL;

	return S_OK;
}

CGigatzoAttackEffect* CGigatzoAttackEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGigatzoAttackEffect* pInstance = new CGigatzoAttackEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGigatzoAttackEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGigatzoAttackEffect::Clone(void* pArg)
{
	CGigatzoAttackEffect* pInstance = new CGigatzoAttackEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGigatzoAttackEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGigatzoAttackEffect::Free()
{
	__super::Free();
}
