#include "BombHitAim.h"
#include "GameContent_const.h"

#include "RectCommon.h"

CBombHitAim::CBombHitAim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CBombHitAim::CBombHitAim(const CBombHitAim& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CBombHitAim::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBombHitAim::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CBombHitAim::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CBombHitAim::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CBombHitAim::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CBombHitAim::Render()
{
	return __super::Render();
}

HRESULT CBombHitAim::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tDesc{};
	tDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_BombHitAim.iLevelID;
	tDesc.wstrTextureTag = Texture_BombHitAim.szProtoTag;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG,
		L"BombHitAim", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CBombHitAim* CBombHitAim::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBombHitAim* pInstance = new CBombHitAim(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CBombHitAim");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBombHitAim::Clone(void* pArg)
{
	CBombHitAim* pInstance = new CBombHitAim(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CBombHitAim");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBombHitAim::Free()
{
	__super::Free();
}
