#include "Armadillo_Dust.h"
#include "GameContent_const.h"

#include "RectParticleCommon.h"

namespace
{
	CRectParticleCommon::RECT_PARTICLE_COMMON_DESC Make_RectDesc(const TEXTURE_DESC& tex)
	{
		CRectParticleCommon::RECT_PARTICLE_COMMON_DESC tRect{};
		tRect.iVIBufferLevel = VI_Rect.iLevelID;
		tRect.wstrVIBufferTag = VI_Rect.szProtoTag;

		tRect.bUseTextureCom = true;
		tRect.iTextureLevel = tex.iLevelID;
		tRect.wstrTextureTag = tex.szProtoTag;

		tRect.bUseMaskCom = false;
		tRect.bCustomShader = false;

		return tRect;
	}
}

CArmadillo_Dust::CArmadillo_Dust(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CArmadillo_Dust::CArmadillo_Dust(const CArmadillo_Dust& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CArmadillo_Dust::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArmadillo_Dust::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CArmadillo_Dust::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CArmadillo_Dust::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CArmadillo_Dust::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CArmadillo_Dust::Render()
{
	return __super::Render();
}

HRESULT CArmadillo_Dust::Ready_EffectPartObjects()
{
	TEXTURE_DESC tTex{};
	tTex.iLevelID = m_iPrototypeLevel;
	tTex.szProtoTag = TEX_PROTOTAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectParticleCommon::PROTOTYPE_TAG, L"Dust", &Make_RectDesc(tTex))))
		return E_FAIL;

	return S_OK;
}

CArmadillo_Dust* CArmadillo_Dust::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CArmadillo_Dust* pInstance = new CArmadillo_Dust(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CArmadillo_Dust");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CArmadillo_Dust::Clone(void* pArg)
{
	CArmadillo_Dust* pInstance = new CArmadillo_Dust(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CArmadillo_Dust");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArmadillo_Dust::Free()
{
	__super::Free();
}
