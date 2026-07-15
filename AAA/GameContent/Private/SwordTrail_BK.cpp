#include "SwordTrail_BK.h"

#include "TrailCommon.h"
#include "GameContent_const.h"

CSwordTrail_BK::CSwordTrail_BK(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSwordTrail_BK::CSwordTrail_BK(const CSwordTrail_BK& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSwordTrail_BK::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSwordTrail_BK::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSwordTrail_BK::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSwordTrail_BK::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSwordTrail_BK::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSwordTrail_BK::Render()
{
	return __super::Render();
}

HRESULT CSwordTrail_BK::Ready_EffectPartObjects()
{
	CTrailCommon::TRAIL_COMMON_DESC tTrailDesc{};
	tTrailDesc.bCustomShader = false;
	tTrailDesc.bUseTextureCom = true;                               
	tTrailDesc.iTextureLevel = Texture_BK_CommonSlash.iLevelID;   
	tTrailDesc.wstrTextureTag = Texture_BK_CommonSlash.szProtoTag;  
	tTrailDesc.bUseMaskCom = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CTrailCommon::PROTOTYPE_TAG, L"Trail", &tTrailDesc)))
		return E_FAIL;

	return S_OK;
}

CSwordTrail_BK* CSwordTrail_BK::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSwordTrail_BK* pInstance = new CSwordTrail_BK(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSwordTrail_BK");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwordTrail_BK::Clone(void* pArg)
{
	CSwordTrail_BK* pInstance = new CSwordTrail_BK(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSwordTrail_BK");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwordTrail_BK::Free()
{
	__super::Free();
}
