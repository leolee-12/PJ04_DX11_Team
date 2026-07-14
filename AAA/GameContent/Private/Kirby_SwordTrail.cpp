#include "Kirby_SwordTrail.h"

#include "GameContent_const.h"

#include "TrailCommon.h"

CKirby_SwordTrail::CKirby_SwordTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CKirby_SwordTrail::CKirby_SwordTrail(const CKirby_SwordTrail& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CKirby_SwordTrail::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKirby_SwordTrail::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CKirby_SwordTrail::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CKirby_SwordTrail::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CKirby_SwordTrail::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CKirby_SwordTrail::Render()
{
	return __super::Render();
}

HRESULT CKirby_SwordTrail::Ready_EffectPartObjects()
{
	CTrailCommon::TRAIL_COMMON_DESC tTrailDesc{};
	tTrailDesc.bCustomShader = false;

	tTrailDesc.bUseTextureCom = true;
	tTrailDesc.iTextureLevel = Texture_SwordTrail.iLevelID;
	tTrailDesc.wstrTextureTag = Texture_SwordTrail.szProtoTag;

	tTrailDesc.bUseMaskCom = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CTrailCommon::PROTOTYPE_TAG, L"Trail", &tTrailDesc)))
		return E_FAIL;

	return S_OK;
}

CKirby_SwordTrail* CKirby_SwordTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKirby_SwordTrail* pInstance = new CKirby_SwordTrail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CKirby_SwordTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKirby_SwordTrail::Clone(void* pArg)
{
	CKirby_SwordTrail* pInstance = new CKirby_SwordTrail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CKirby_SwordTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKirby_SwordTrail::Free()
{
	__super::Free();
}
