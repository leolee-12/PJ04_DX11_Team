#include "DropStarEffect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "RectCommon.h"

namespace
{
	CRectCommon::RECT_COMMON_DESC Make_RectDesc(const TEXTURE_DESC& tex)
	{
		CRectCommon::RECT_COMMON_DESC tRect{};
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

CDropStarEffect::CDropStarEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CDropStarEffect::CDropStarEffect(const CDropStarEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CDropStarEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDropStarEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CDropStarEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CDropStarEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CDropStarEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CDropStarEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDropStarEffect::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tRing = Make_RectDesc(Texture_CommonRing01);

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"RingHalo", &tRing)))
		return E_FAIL;

	return S_OK;
}

CDropStarEffect* CDropStarEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDropStarEffect* pInstance = new CDropStarEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDropStarEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDropStarEffect::Clone(void* pArg)
{
	CDropStarEffect* pInstance = new CDropStarEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDropStarEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDropStarEffect::Free()
{
	__super::Free();
}