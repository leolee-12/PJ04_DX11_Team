#include "Env_SpotLight.h"

NS_BEGIN(Client)

CEnv_SpotLight::CEnv_SpotLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
	, m_vDirection{}
	, m_vColor{}
	, m_fIntensity{ 0.f }
	, m_fRange{ 0.f }
	, m_fAngle{ 0.f }
	, m_fDecayStartAngle{ 0.f }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnv_SpotLight::CEnv_SpotLight(const CEnv_SpotLight& Prototype)
	: CEnvObject(Prototype)
	, m_vDirection{ Prototype.m_vDirection }
	, m_vColor{ Prototype.m_vColor }
	, m_fIntensity{ Prototype.m_fIntensity }
	, m_fRange{ Prototype.m_fRange }
	, m_fAngle{ Prototype.m_fAngle }
	, m_fDecayStartAngle{ Prototype.m_fDecayStartAngle }
	, m_strKind{ Prototype.m_strKind }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnv_SpotLight::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnv_SpotLight::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_vDirection = m_tDesc.tEffect.vDirection;
	m_vColor = m_tDesc.tEffect.vColor;
	m_fIntensity = m_tDesc.tEffect.fIntensity;
	m_fRange = m_tDesc.tEffect.fRange;
	m_fAngle = m_tDesc.tEffect.fAngle;
	m_fDecayStartAngle = m_tDesc.tEffect.fDecayStartAngle;
	m_strKind = m_tDesc.tEffect.strKind;

	m_bRenderable = false;
	m_bCastShadow = false;
	m_bUseCullDistance = false;
	m_bUseCullFrustum = false;

	return S_OK;
}

void CEnv_SpotLight::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

CEnv_SpotLight* CEnv_SpotLight::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnv_SpotLight* pInstance = new CEnv_SpotLight(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnv_SpotLight");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnv_SpotLight::Clone(void* pArg)
{
	CEnv_SpotLight* pInstance = new CEnv_SpotLight(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnv_SpotLight");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnv_SpotLight::Free()
{
	__super::Free();
}

NS_END