#include "EnvTrigger_RenderGlobals.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CEnvTrigger_RenderGlobals::CEnvTrigger_RenderGlobals(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Trigger(pDevice, pContext)
	, m_fInTransitionSec{ 0.f }
	, m_fOutTransitionSec{ 0.f }
	, m_fSSAORadius{ 5.f }
	, m_fSSAOBias{ 0.1f }
	, m_fSSAOPower{ 2.4f }
	, m_fBloomThreshold{ 1.f }
	, m_fBloomIntensity{ 1.f }
	, m_fSSRIntensity{ 1.f }
	, m_fSSRMaxDistance{ 30.f }
	, m_fSSRThickness{ 0.5f }
	, m_bDoFEnable{ true }
	, m_fFocusDist{ 14.f }
	, m_fAperture{ 0.1f }
	, m_fDoFMaxCoC{ 1.f }
	, m_bDoFAutoFocus{ false }
	, m_fExposure{ 1.f }
	, m_fToneMapMode{ 0.f }
	, m_fAmbientSaturation{ 0.6f }
	, m_bFogEnable{ true }
	, m_fFogDensity{ 0.04f }
	, m_vFogColor{ 0.70f, 0.80f, 1.00f }
	, m_fFogFar{ 80.f }
	, m_fFogStart{ 30.f }
	, m_fFogStartRange{ 70.f }
	, m_fFogHeightFalloff{ 0.04f }
	, m_fFogBaseHeight{ 0.f }
	, m_fFogAnisotropy{ 0.70f }
	, m_fFogAmbient{ 0.02f }
	, m_fFogShadowStrength{ 0.f }
	, m_fFogLightIntensity{ 3.f }
	, m_vAtmosColor{ 0.70f, 0.75f, 0.82f }
	, m_fAtmosStart{ 50.f }
	, m_fAtmosEnd{ 300.f }
	, m_fAtmosStrength{ 0.5f }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvTrigger_RenderGlobals::CEnvTrigger_RenderGlobals(const CEnvTrigger_RenderGlobals& Prototype)
	: CEnvObject_Trigger(Prototype)
	, m_fInTransitionSec{ Prototype.m_fInTransitionSec }
	, m_fOutTransitionSec{ Prototype.m_fOutTransitionSec }
	, m_fSSAORadius{ Prototype.m_fSSAORadius }
	, m_fSSAOBias{ Prototype.m_fSSAOBias }
	, m_fSSAOPower{ Prototype.m_fSSAOPower }
	, m_fBloomThreshold{ Prototype.m_fBloomThreshold }
	, m_fBloomIntensity{ Prototype.m_fBloomIntensity }
	, m_fSSRIntensity{ Prototype.m_fSSRIntensity }
	, m_fSSRMaxDistance{ Prototype.m_fSSRMaxDistance }
	, m_fSSRThickness{ Prototype.m_fSSRThickness }
	, m_bDoFEnable{ Prototype.m_bDoFEnable }
	, m_fFocusDist{ Prototype.m_fFocusDist }
	, m_fAperture{ Prototype.m_fAperture }
	, m_fDoFMaxCoC{ Prototype.m_fDoFMaxCoC }
	, m_bDoFAutoFocus{ Prototype.m_bDoFAutoFocus }
	, m_fExposure{ Prototype.m_fExposure }
	, m_fToneMapMode{ Prototype.m_fToneMapMode }
	, m_fAmbientSaturation{ Prototype.m_fAmbientSaturation }
	, m_bFogEnable{ Prototype.m_bFogEnable }
	, m_fFogDensity{ Prototype.m_fFogDensity }
	, m_vFogColor{ Prototype.m_vFogColor }
	, m_fFogFar{ Prototype.m_fFogFar }
	, m_fFogStart{ Prototype.m_fFogStart }
	, m_fFogStartRange{ Prototype.m_fFogStartRange }
	, m_fFogHeightFalloff{ Prototype.m_fFogHeightFalloff }
	, m_fFogBaseHeight{ Prototype.m_fFogBaseHeight }
	, m_fFogAnisotropy{ Prototype.m_fFogAnisotropy }
	, m_fFogAmbient{ Prototype.m_fFogAmbient }
	, m_fFogShadowStrength{ Prototype.m_fFogShadowStrength }
	, m_fFogLightIntensity{ Prototype.m_fFogLightIntensity }
	, m_vAtmosColor{ Prototype.m_vAtmosColor }
	, m_fAtmosStart{ Prototype.m_fAtmosStart }
	, m_fAtmosEnd{ Prototype.m_fAtmosEnd }
	, m_fAtmosStrength{ Prototype.m_fAtmosStrength }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvTrigger_RenderGlobals::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvTrigger_RenderGlobals::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (m_tDesc.tEffect.eEffectType == ENV_EFFECT_TYPE::TONE_MAPPING_AREA)
	{
		m_fExposure = m_tDesc.tEffect.fExposureValue;
		m_fInTransitionSec = m_tDesc.tEffect.fInTransitionSec > 0.f ? m_tDesc.tEffect.fInTransitionSec : m_tDesc.tEffect.fTransitionSec;
		m_fOutTransitionSec = m_tDesc.tEffect.fOutTransitionSec > 0.f ? m_tDesc.tEffect.fOutTransitionSec : m_tDesc.tEffect.fTransitionSec;
	}

	return S_OK;
}

void CEnvTrigger_RenderGlobals::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

void CEnvTrigger_RenderGlobals::OnTriggerEnter(CCollider* pOther)
{
	if (false == Is_TriggerActivator(pOther))
		return;

	Apply_RenderGlobals();

	OutputDebugStringA("[EnvTrigger_RenderGlobals] Apply RenderGlobals\n");
}

void CEnvTrigger_RenderGlobals::OnTriggerStay(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
}

void CEnvTrigger_RenderGlobals::OnTriggerExit(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
}

void CEnvTrigger_RenderGlobals::Apply_RenderGlobals()
{
	const auto SetBool = [this](const string& strName, _bool bValue)
		{
			m_pGameInstance_Proxy->Tween_ShaderGlobal(strName, _float4(bValue ? 1.f : 0.f, 0.f, 0.f, 0.f), m_fInTransitionSec);
		};

	const auto SetFloat = [this](const string& strName, _float fValue)
		{
			m_pGameInstance_Proxy->Tween_ShaderGlobal(strName, _float4(fValue, 0.f, 0.f, 0.f), m_fInTransitionSec);
		};

	const auto SetFloat3 = [this](const string& strName, const _float3& vValue)
		{
			m_pGameInstance_Proxy->Tween_ShaderGlobal(strName, _float4(vValue.x, vValue.y, vValue.z, 0.f), m_fInTransitionSec);
		};

	SetFloat("g_fSSAORadius", m_fSSAORadius);
	SetFloat("g_fSSAOBias", m_fSSAOBias);
	SetFloat("g_fSSAOPower", m_fSSAOPower);

	SetFloat("g_fThreshold", m_fBloomThreshold);
	SetFloat("g_fBloomIntensity", m_fBloomIntensity);

	SetFloat("g_fSSRIntensity", m_fSSRIntensity);
	SetFloat("g_fSSRMaxDistance", m_fSSRMaxDistance);
	SetFloat("g_fSSRThickness", m_fSSRThickness);

	SetBool("g_fDoFEnable", m_bDoFEnable);
	SetFloat("g_fFocusDist", m_fFocusDist);
	SetFloat("g_fAperture", m_fAperture);
	SetFloat("g_fDoFMaxCoC", m_fDoFMaxCoC);
	SetBool("g_fDoFAutoFocus", m_bDoFAutoFocus);

	SetFloat("g_fExposure", m_fExposure);
	SetFloat("g_fToneMapMode", m_fToneMapMode);
	SetFloat("g_fAmbientSaturation", m_fAmbientSaturation);

	SetBool("g_fFogEnable", m_bFogEnable);
	SetFloat("g_fFogDensity", m_fFogDensity);
	SetFloat3("g_vFogColor", m_vFogColor);
	SetFloat("g_fFogFar", m_fFogFar);
	SetFloat("g_fFogStart", m_fFogStart);
	SetFloat("g_fFogStartRange", m_fFogStartRange);
	SetFloat("g_fFogHeightFalloff", m_fFogHeightFalloff);
	SetFloat("g_fFogBaseHeight", m_fFogBaseHeight);
	SetFloat("g_fFogAnisotropy", m_fFogAnisotropy);
	SetFloat("g_fFogAmbient", m_fFogAmbient);
	SetFloat("g_fFogShadowStrength", m_fFogShadowStrength);
	SetFloat("g_fFogLightIntensity", m_fFogLightIntensity);

	SetFloat3("g_vAtmosColor", m_vAtmosColor);
	SetFloat("g_fAtmosStart", m_fAtmosStart);
	SetFloat("g_fAtmosEnd", m_fAtmosEnd);
	SetFloat("g_fAtmosStrength", m_fAtmosStrength);
}

_bool CEnvTrigger_RenderGlobals::Is_TriggerActivator(CCollider* pOther) const
{
	return nullptr != pOther && ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup();
}

CEnvTrigger_RenderGlobals* CEnvTrigger_RenderGlobals::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvTrigger_RenderGlobals* pInstance = new CEnvTrigger_RenderGlobals(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvTrigger_RenderGlobals");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvTrigger_RenderGlobals::Clone(void* pArg)
{
	CEnvTrigger_RenderGlobals* pInstance = new CEnvTrigger_RenderGlobals(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvTrigger_RenderGlobals");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvTrigger_RenderGlobals::Free()
{
	__super::Free();
}

NS_END