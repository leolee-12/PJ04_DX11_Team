#include "HammerFireSwing.h"

#include "GameContent_const.h"
#include "RectEmitterCommon.h"

CHammerFireSwing::CHammerFireSwing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CHammerFireSwing::CHammerFireSwing(const CHammerFireSwing& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CHammerFireSwing::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHammerFireSwing::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CHammerFireSwing::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CHammerFireSwing::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CHammerFireSwing::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CHammerFireSwing::Render()
{
	return __super::Render();
}

HRESULT CHammerFireSwing::Ready_EffectPartObjects()
{
	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tDesc{};
	tDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_HammerFireSwing.iLevelID;
	tDesc.wstrTextureTag = Texture_HammerFireSwing.szProtoTag;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, MAIN_PART_TAG, &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, CORE_PART_TAG, &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, ACCENT_PART_TAG, &tDesc)))
		return E_FAIL;

	return S_OK;
}

CHammerFireSwing* CHammerFireSwing::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHammerFireSwing* pInstance = new CHammerFireSwing(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CHammerFireSwing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHammerFireSwing::Clone(void* pArg)
{
	CHammerFireSwing* pInstance = new CHammerFireSwing(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CHammerFireSwing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHammerFireSwing::Free()
{
	__super::Free();
}
