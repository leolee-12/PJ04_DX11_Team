#include "Nail_Trail.h"

#include "TrailCommon.h"
#include "GameContent_const.h"

CNail_Trail::CNail_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CNail_Trail::CNail_Trail(const CNail_Trail& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CNail_Trail::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNail_Trail::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CNail_Trail::Ready_EffectPartObjects()
{
	CTrailCommon::TRAIL_COMMON_DESC tTrailDesc{};
	tTrailDesc.bCustomShader = false;
	tTrailDesc.bUseTextureCom = true;
	tTrailDesc.iTextureLevel = Texture_NailTrail.iLevelID;
	tTrailDesc.wstrTextureTag = Texture_NailTrail.szProtoTag;
	tTrailDesc.bUseMaskCom = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CTrailCommon::PROTOTYPE_TAG, L"Trail", &tTrailDesc)))
		return E_FAIL;

	return S_OK;
}

CNail_Trail* CNail_Trail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNail_Trail* pInstance = new CNail_Trail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CNail_Trail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNail_Trail::Clone(void* pArg)
{
	CNail_Trail* pInstance = new CNail_Trail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CNail_Trail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNail_Trail::Free()
{
	__super::Free();
}
