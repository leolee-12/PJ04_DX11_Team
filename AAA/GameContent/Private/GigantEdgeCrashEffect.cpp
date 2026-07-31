#include "GigantEdgeCrashEffect.h"

#include "MeshParticleCommon.h"
#include "RectCommon.h"
#include "GameContent_const.h"

CGigantEdgeCrashEffect::CGigantEdgeCrashEffect(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CGigantEdgeCrashEffect::CGigantEdgeCrashEffect(
	const CGigantEdgeCrashEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CGigantEdgeCrashEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGigantEdgeCrashEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CGigantEdgeCrashEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGigantEdgeCrashEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CGigantEdgeCrashEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CGigantEdgeCrashEffect::Render()
{
	return __super::Render();
}

HRESULT CGigantEdgeCrashEffect::Ready_EffectPartObjects()
{
	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tRockDesc{};
	tRockDesc.iModelLevel = m_iPrototypeLevel;
	tRockDesc.wstrModelTag = FLOOR_MODEL_TAG;
	tRockDesc.bUseDiffuseTexture = true;
	tRockDesc.bUseNormalTexture = true;
	tRockDesc.bUseMRATexture = true;
	tRockDesc.bUseUnknownTexture = false;
	tRockDesc.bUseTextureCom = false;
	tRockDesc.bUseMaskCom = false;
	tRockDesc.bCustomShader = true;
	tRockDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tRockDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshParticleCommon::PROTOTYPE_TAG,
		L"Rock", &tRockDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshParticleCommon::PROTOTYPE_TAG,
		L"RockS", &tRockDesc)))
		return E_FAIL;

	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tSmokeDesc{};
	tSmokeDesc.iModelLevel = m_iPrototypeLevel;
	tSmokeDesc.wstrModelTag = SMOKE_MODEL_TAG;
	tSmokeDesc.bUseDiffuseTexture = false;
	tSmokeDesc.bUseNormalTexture = false;
	tSmokeDesc.bUseMRATexture = false;
	tSmokeDesc.bUseUnknownTexture = true;
	tSmokeDesc.bUseTextureCom = false;
	tSmokeDesc.bUseMaskCom = false;
	tSmokeDesc.bCustomShader = true;
	tSmokeDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tSmokeDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CMeshParticleCommon::PROTOTYPE_TAG,
		L"Smoke", &tSmokeDesc)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tFlashDesc{};
	tFlashDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tFlashDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tFlashDesc.bUseTextureCom = true;
	tFlashDesc.iTextureLevel = Texture_GE_CircleGlow.iLevelID;
	tFlashDesc.wstrTextureTag = Texture_GE_CircleGlow.szProtoTag;
	tFlashDesc.bUseMaskCom = false;
	tFlashDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CRectCommon::PROTOTYPE_TAG,
		L"Flash", &tFlashDesc)))
		return E_FAIL;

	return S_OK;
}

CGigantEdgeCrashEffect* CGigantEdgeCrashEffect::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CGigantEdgeCrashEffect* pInstance =
		new CGigantEdgeCrashEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CGigantEdgeCrashEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGigantEdgeCrashEffect::Clone(void* pArg)
{
	CGigantEdgeCrashEffect* pInstance =
		new CGigantEdgeCrashEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CGigantEdgeCrashEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGigantEdgeCrashEffect::Free()
{
	__super::Free();
}
