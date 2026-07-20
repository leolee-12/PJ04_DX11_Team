#include "OnLadderEffect.h"
#include "GameContent_const.h"

#include "RectCommon.h"

COnLadderEffect::COnLadderEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

COnLadderEffect::COnLadderEffect(const COnLadderEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT COnLadderEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT COnLadderEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void COnLadderEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void COnLadderEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void COnLadderEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT COnLadderEffect::Render()
{
	return __super::Render();
}

HRESULT COnLadderEffect::Ready_EffectPartObjects()
{
	CRectCommon::RECT_COMMON_DESC tDesc{};
	tDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Star2D.iLevelID;
	tDesc.wstrTextureTag = Texture_Star2D.szProtoTag;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG,
		L"Star00", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectCommon::PROTOTYPE_TAG,
		L"Star01", &tDesc)))
		return E_FAIL;

	return S_OK;
}

COnLadderEffect* COnLadderEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COnLadderEffect* pInstance = new COnLadderEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: COnLadderEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* COnLadderEffect::Clone(void* pArg)
{
	COnLadderEffect* pInstance = new COnLadderEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: COnLadderEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void COnLadderEffect::Free()
{
	__super::Free();
}
