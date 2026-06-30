#include "EnvTrigger_RenderGlobals.h"

#include "GameInstance.h"

namespace
{
	const char* RENDER_GLOBAL_NAMES[] =
	{
			"g_fSSAORadius",
			"g_fSSAOBias",
			"g_fSSAOPower",
			"g_fThreshold",
			"g_fBloomIntensity",
			"g_fSSRIntensity",
			"g_fSSRMaxDistance",
			"g_fSSRThickness",
			"g_fDoFEnable",
			"g_fFocusDist",
			"g_fAperture",
			"g_fDoFMaxCoC",
			"g_fDoFAutoFocus",
			"g_fExposure",
			"g_fToneMapMode",
			"g_fAmbientSaturation",
			"g_fFogEnable",
			"g_fFogDensity",
			"g_vFogColor",
			"g_fFogFar",
			"g_fFogStart",
			"g_fFogStartRange",
			"g_fFogHeightFalloff",
			"g_fFogBaseHeight",
			"g_fFogAnisotropy",
			"g_fFogAmbient",
			"g_fFogShadowStrength",
			"g_fFogLightIntensity",
			"g_vAtmosColor",
			"g_fAtmosStart",
			"g_fAtmosEnd",
			"g_fAtmosStrength",
	};

	vector<Client::CEnvTrigger_RenderGlobals*> g_ActiveRenderGlobalTriggers;
	unordered_map<string, _float4> g_BaseRenderGlobals;
	Client::CEnvTrigger_RenderGlobals* g_pAppliedRenderGlobalTrigger = { nullptr };
}

NS_BEGIN(Client)

CEnvTrigger_RenderGlobals::CEnvTrigger_RenderGlobals(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Trigger(pDevice, pContext)
	, m_iPriority{ 0 }
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
	, m_iPriority{ Prototype.m_iPriority }
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
	if (false == Is_TriggerActivator(pOther) || true == m_bActiveRenderGlobals)
		return;

	Activate_RenderGlobals();

	OutputDebugStringA("[EnvTrigger_RenderGlobals] Activate RenderGlobals\n");
}

void CEnvTrigger_RenderGlobals::OnTriggerStay(CCollider* pOther)
{
	if (false == Is_TriggerActivator(pOther) || false == m_bActiveRenderGlobals)
		return;

	if (Get_ActiveRenderGlobals() != this)
		return;

	Apply_RenderGlobals();
}

void CEnvTrigger_RenderGlobals::OnTriggerExit(CCollider* pOther)
{
	if (false == Is_TriggerActivator(pOther) || false == m_bActiveRenderGlobals)
		return;

	Deactivate_RenderGlobals();

	OutputDebugStringA("[EnvTrigger_RenderGlobals] Deactivate RenderGlobals\n");
}

void CEnvTrigger_RenderGlobals::Apply_RenderGlobals()
{
	const auto SetBool = [this](const string& strName, _bool bValue)
		{
			m_pGameInstance_Proxy->Set_ShaderGlobal(strName, _float4(bValue ? 1.f : 0.f, 0.f, 0.f, 0.f));
		};

	const auto SetFloat = [this](const string& strName, _float fValue)
		{
			m_pGameInstance_Proxy->Set_ShaderGlobal(strName, _float4(fValue, 0.f, 0.f, 0.f));
		};

	const auto SetFloat3 = [this](const string& strName, const _float3& vValue)
		{
			m_pGameInstance_Proxy->Set_ShaderGlobal(strName, _float4(vValue.x, vValue.y, vValue.z, 0.f));
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

void CEnvTrigger_RenderGlobals::Activate_RenderGlobals()
{
	if (g_ActiveRenderGlobalTriggers.empty())
		Save_BaseRenderGlobals();

	g_ActiveRenderGlobalTriggers.push_back(this);
	m_bActiveRenderGlobals = true;

	Resolve_RenderGlobals();
}

void CEnvTrigger_RenderGlobals::Deactivate_RenderGlobals()
{
	for (auto iter = g_ActiveRenderGlobalTriggers.begin(); iter != g_ActiveRenderGlobalTriggers.end(); ++iter)
	{
		if (*iter == this)
		{
			g_ActiveRenderGlobalTriggers.erase(iter);
			break;
		}
	}

	if (g_pAppliedRenderGlobalTrigger == this)
		g_pAppliedRenderGlobalTrigger = nullptr;

	m_bActiveRenderGlobals = false;

	Resolve_RenderGlobals();
}

void CEnvTrigger_RenderGlobals::Save_BaseRenderGlobals()
{
	g_BaseRenderGlobals.clear();

	for (const char* pName : RENDER_GLOBAL_NAMES)
	{
		const _float4* pValue = m_pGameInstance_Proxy->Get_ShaderGlobal(pName);
		if (nullptr != pValue)
			g_BaseRenderGlobals.emplace(pName, *pValue);
	}
}

void CEnvTrigger_RenderGlobals::Restore_BaseRenderGlobals()
{
	for (const auto& Pair : g_BaseRenderGlobals)
		m_pGameInstance_Proxy->Set_ShaderGlobal(Pair.first, Pair.second);

	g_BaseRenderGlobals.clear();
}

void CEnvTrigger_RenderGlobals::Resolve_RenderGlobals()
{
	CEnvTrigger_RenderGlobals* pActiveTrigger = Get_ActiveRenderGlobals();

	if (nullptr == pActiveTrigger)
	{
		Restore_BaseRenderGlobals();
		g_pAppliedRenderGlobalTrigger = nullptr;
		return;
	}

	if (g_pAppliedRenderGlobalTrigger == pActiveTrigger)
		return;

	pActiveTrigger->Apply_RenderGlobals();
	g_pAppliedRenderGlobalTrigger = pActiveTrigger;
}

CEnvTrigger_RenderGlobals* CEnvTrigger_RenderGlobals::Get_ActiveRenderGlobals()
{
	CEnvTrigger_RenderGlobals* pActiveTrigger = nullptr;

	for (CEnvTrigger_RenderGlobals* pTrigger : g_ActiveRenderGlobalTriggers)
	{
		if (nullptr == pTrigger)
			continue;

		if (nullptr == pActiveTrigger || pActiveTrigger->m_iPriority <= pTrigger->m_iPriority)
			pActiveTrigger = pTrigger;
	}

	return pActiveTrigger;
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
	if (true == m_bActiveRenderGlobals)
		Deactivate_RenderGlobals();

	__super::Free();
}

NS_END