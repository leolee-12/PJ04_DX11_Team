#include "BombAimDot.h"
#include "GameContent_const.h"

#include "RectCommon.h"

CBombAimDot::CBombAimDot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CBombAimDot::CBombAimDot(const CBombAimDot& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CBombAimDot::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBombAimDot::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CBombAimDot::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CBombAimDot::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CBombAimDot::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CBombAimDot::Render()
{
	return __super::Render();
}

HRESULT CBombAimDot::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tDesc{};
	tDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_BombAimDot.iLevelID;
	tDesc.wstrTextureTag = Texture_BombAimDot.szProtoTag;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG,
		L"BombAimDot", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CBombAimDot* CBombAimDot::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBombAimDot* pInstance = new CBombAimDot(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CBombAimDot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBombAimDot::Clone(void* pArg)
{
	CBombAimDot* pInstance = new CBombAimDot(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CBombAimDot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBombAimDot::Free()
{
	__super::Free();
}
