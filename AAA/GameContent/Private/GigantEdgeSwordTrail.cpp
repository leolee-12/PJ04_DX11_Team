#include "GigantEdgeSwordTrail.h"

#include "TrailCommon.h"
#include "GameContent_const.h"

CGigantEdgeSwordTrail::CGigantEdgeSwordTrail(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CGigantEdgeSwordTrail::CGigantEdgeSwordTrail(
	const CGigantEdgeSwordTrail& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CGigantEdgeSwordTrail::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGigantEdgeSwordTrail::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CGigantEdgeSwordTrail::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGigantEdgeSwordTrail::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CGigantEdgeSwordTrail::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CGigantEdgeSwordTrail::Render()
{
	return __super::Render();
}

HRESULT CGigantEdgeSwordTrail::Ready_EffectPartObjects()
{
	CTrailCommon::TRAIL_COMMON_DESC tTrailDesc{};
	tTrailDesc.bCustomShader = false;
	tTrailDesc.bUseTextureCom = true;
	tTrailDesc.iTextureLevel = Texture_GE_Slash.iLevelID;
	tTrailDesc.wstrTextureTag = Texture_GE_Slash.szProtoTag;
	tTrailDesc.bUseMaskCom = false;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CTrailCommon::PROTOTYPE_TAG,
		L"Trail",
		&tTrailDesc)))
		return E_FAIL;

	return S_OK;
}

CGigantEdgeSwordTrail* CGigantEdgeSwordTrail::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CGigantEdgeSwordTrail* pInstance =
		new CGigantEdgeSwordTrail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CGigantEdgeSwordTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGigantEdgeSwordTrail::Clone(void* pArg)
{
	CGigantEdgeSwordTrail* pInstance =
		new CGigantEdgeSwordTrail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CGigantEdgeSwordTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGigantEdgeSwordTrail::Free()
{
	__super::Free();
}
