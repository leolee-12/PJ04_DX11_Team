#include "WarpOutEnd.h"
#include "GameContent_const.h"

#include "RectCommon.h"
#include "MeshParticleCommon.h"

CWarpOutEnd::CWarpOutEnd(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CWarpOutEnd::CWarpOutEnd(const CWarpOutEnd& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CWarpOutEnd::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWarpOutEnd::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CWarpOutEnd::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CWarpOutEnd::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CWarpOutEnd::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CWarpOutEnd::Render()
{
	return __super::Render();
}

HRESULT CWarpOutEnd::Ready_EffectPartObjects()
{
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

CWarpOutEnd* CWarpOutEnd::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CWarpOutEnd* pInstance = new CWarpOutEnd(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CWarpOutEnd");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWarpOutEnd::Clone(void* pArg)
{
	CWarpOutEnd* pInstance = new CWarpOutEnd(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CWarpOutEnd");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWarpOutEnd::Free()
{
	__super::Free();
}
