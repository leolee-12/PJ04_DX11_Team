#include "WarpInEffect.h"
#include "GameContent_const.h"

#include "RectParticleCommon.h"
#include "RectCommon.h"
#include "MeshParticleCommon.h"

CWarpInEffect::CWarpInEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CWarpInEffect::CWarpInEffect(const CWarpInEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CWarpInEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWarpInEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CWarpInEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CWarpInEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CWarpInEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CWarpInEffect::Render()
{
	return __super::Render();
}

HRESULT CWarpInEffect::Ready_EffectPartObjects()
{
	CRectParticleCommon::RECT_PARTICLE_COMMON_DESC tLineDesc{};
	tLineDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tLineDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tLineDesc.bUseTextureCom = true;
	tLineDesc.iTextureLevel = Texture_Kabu_CommonLine.iLevelID;
	tLineDesc.wstrTextureTag = Texture_Kabu_CommonLine.szProtoTag;
	tLineDesc.bUseMaskCom = false;
	tLineDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectParticleCommon::PROTOTYPE_TAG,
		L"LineParticle", &tLineDesc)))
		return E_FAIL;

	CRectCommon::RECT_COMMON_DESC tFlashDesc{};
	tFlashDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tFlashDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tFlashDesc.bUseTextureCom = true;
	tFlashDesc.iTextureLevel = Texture_Kabu_FlashCircle.iLevelID;
	tFlashDesc.wstrTextureTag = Texture_Kabu_FlashCircle.szProtoTag;
	tFlashDesc.bUseMaskCom = false;
	tFlashDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG,
		L"Flash", &tFlashDesc)))
		return E_FAIL;

	CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tSmokeDesc{};
	tSmokeDesc.iModelLevel = m_iPrototypeLevel;
	tSmokeDesc.wstrModelTag = SMOKE_MODEL_PROTO_TAG;
	tSmokeDesc.bUseDiffuseTexture = false;
	tSmokeDesc.bUseUnknownTexture = true;
	tSmokeDesc.bUseNormalTexture = false;
	tSmokeDesc.bUseMRATexture = false;
	tSmokeDesc.bUseTextureCom = false;
	tSmokeDesc.bUseMaskCom = false;
	tSmokeDesc.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG,
		L"Smoke", &tSmokeDesc)))
		return E_FAIL;

	return S_OK;
}

CWarpInEffect* CWarpInEffect::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CWarpInEffect* pInstance = new CWarpInEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CWarpInEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWarpInEffect::Clone(void* pArg)
{
	CWarpInEffect* pInstance = new CWarpInEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CWarpInEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWarpInEffect::Free()
{
	__super::Free();
}
