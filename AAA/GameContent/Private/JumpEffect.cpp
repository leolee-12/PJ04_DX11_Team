#include "JumpEffect.h"
#include "GameContent_const.h"

#include "RectCommon.h"

CJumpEffect::CJumpEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CJumpEffect::CJumpEffect(const CJumpEffect& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CJumpEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CJumpEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CJumpEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CJumpEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CJumpEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CJumpEffect::Render()
{
	return __super::Render();
}

HRESULT CJumpEffect::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tRectDesc{};
	tRectDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tRectDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tRectDesc.bUseTextureCom = true;
	tRectDesc.iTextureLevel = Texture_JumpEffect.iLevelID;
	tRectDesc.wstrTextureTag = Texture_JumpEffect.szProtoTag;
	tRectDesc.bUseMaskCom = false;
	tRectDesc.bCustomShader = false;

	static constexpr const _tchar* PART_TAGS[] =
	{
		L"JumpEffect01",
		L"JumpEffect02",
		L"JumpEffect03",
		L"JumpEffect04",
		L"JumpEffect05",
	};

	for (const _tchar* pPartTag : PART_TAGS)
	{
		if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
			CRectCommon::PROTOTYPE_TAG, pPartTag, &tRectDesc)))
			return E_FAIL;
	}

	return S_OK;
}

CJumpEffect* CJumpEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CJumpEffect* pInstance = new CJumpEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CJumpEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CJumpEffect::Clone(void* pArg)
{
	CJumpEffect* pInstance = new CJumpEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CJumpEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CJumpEffect::Free()
{
	__super::Free();
}
