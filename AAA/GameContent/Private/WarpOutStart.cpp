#include "WarpOutStart.h"
#include "GameContent_const.h"

#include "RectParticleCommon.h"

CWarpOutStart::CWarpOutStart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CWarpOutStart::CWarpOutStart(const CWarpOutStart& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CWarpOutStart::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWarpOutStart::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CWarpOutStart::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CWarpOutStart::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CWarpOutStart::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CWarpOutStart::Render()
{
	return __super::Render();
}

HRESULT CWarpOutStart::Ready_EffectPartObjects()
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

	return S_OK;
}

CWarpOutStart* CWarpOutStart::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CWarpOutStart* pInstance = new CWarpOutStart(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CWarpOutStart");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWarpOutStart::Clone(void* pArg)
{
	CWarpOutStart* pInstance = new CWarpOutStart(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CWarpOutStart");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWarpOutStart::Free()
{
	__super::Free();
}
