#include "ItemEffect.h"
#include "GameContent_const.h"
#include "RectCommon.h"
#include "RectEmitterCommon.h"

CItemEffect::CItemEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CItemEffect::CItemEffect(const CItemEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CItemEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CItemEffect::Initialize(void* pArg)
{
	EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CItemEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CItemEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CItemEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CItemEffect::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CItemEffect::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tRect{};
	tRect.iVIBufferLevel = VI_Rect.iLevelID;
	tRect.wstrVIBufferTag = VI_Rect.szProtoTag;
	tRect.bUseTextureCom = true;
	tRect.iTextureLevel = Texture_ItemCircle.iLevelID;
	tRect.wstrTextureTag = Texture_ItemCircle.szProtoTag;
	tRect.bUseMaskCom = false;
	tRect.bCustomShader = false;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG, L"Halo", &tRect)))
		return E_FAIL;

	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tRectEmitDesc{};
	tRectEmitDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tRectEmitDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tRectEmitDesc.bUseTextureCom = true;
	tRectEmitDesc.bUseMaskCom = false;
	tRectEmitDesc.bCustomShader = false;

	tRectEmitDesc.iTextureLevel = Texture_ItemSparkle01.iLevelID;
	tRectEmitDesc.wstrTextureTag = Texture_ItemSparkle01.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Sparkle01"), &tRectEmitDesc)))
		return E_FAIL;

	tRectEmitDesc.iTextureLevel = Texture_ItemSparkle02.iLevelID;
	tRectEmitDesc.wstrTextureTag = Texture_ItemSparkle02.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Sparkle02"), &tRectEmitDesc)))
		return E_FAIL;

	tRectEmitDesc.iTextureLevel = Texture_ItemSparkle03.iLevelID;
	tRectEmitDesc.wstrTextureTag = Texture_ItemSparkle03.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Sparkle03"), &tRectEmitDesc)))
		return E_FAIL;

	return S_OK;
}

CItemEffect* CItemEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CItemEffect* pInstance = new CItemEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CItemEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CItemEffect::Clone(void* pArg)
{
	CItemEffect* pInstance = new CItemEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CItemEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CItemEffect::Free()
{
	__super::Free();
}
