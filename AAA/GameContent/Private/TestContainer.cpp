#include "TestContainer.h"

#include "DistortionCommon.h"
#include "GameContent_const.h"

CTestContainer::CTestContainer(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CTestContainer::CTestContainer(const CTestContainer& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CTestContainer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTestContainer::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CTestContainer::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CTestContainer::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CTestContainer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTestContainer::Render()
{
	return __super::Render();
}

HRESULT CTestContainer::Ready_EffectPartObjects()
{
	CDistortionCommon::DISTORTION_COMMON_DESC DistortionDesc{};
	DistortionDesc.iModelLevel = m_iPrototypeLevel;
	DistortionDesc.wstrModelTag = MODEL_PROTO_TAG;
	DistortionDesc.bUseTextureCom = true;
	DistortionDesc.iTextureLevel = Texture_ChargeNoise.iLevelID;
	DistortionDesc.wstrTextureTag = Texture_ChargeNoise.szProtoTag;

	if (FAILED(Add_Effect_PartObject(
		m_iPrototypeLevel,
		CDistortionCommon::PROTOTYPE_TAG,
		PART_TAG,
		&DistortionDesc)))
		return E_FAIL;

	return S_OK;
}

CTestContainer* CTestContainer::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CTestContainer* pInstance = new CTestContainer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CTestContainer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTestContainer::Clone(void* pArg)
{
	CTestContainer* pInstance = new CTestContainer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CTestContainer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTestContainer::Free()
{
	__super::Free();
}
