#pragma once
#include "EnvObject_Trigger.h"

NS_BEGIN(Client)

class CLIENT_DLL CEnvTrigger_RenderGlobals final : public CEnvObject_Trigger
{
	GENERATED_BODY(CEnvTrigger_RenderGlobals)

	PROPERTY(_float, m_fInTransitionSec, L"In Transition Sec", L"Render Globals")
	PROPERTY(_float, m_fOutTransitionSec, L"Out Transition Sec", L"Render Globals")

	PROPERTY(_float, m_fSSAORadius, L"SSAO Radius", L"SSAO")
	PROPERTY(_float, m_fSSAOBias, L"SSAO Bias", L"SSAO")
	PROPERTY(_float, m_fSSAOPower, L"SSAO Power", L"SSAO")

	PROPERTY(_float, m_fBloomThreshold, L"Bloom Threshold", L"Bloom")
	PROPERTY(_float, m_fBloomIntensity, L"Bloom Intensity", L"Bloom")

	PROPERTY(_float, m_fSSRIntensity, L"SSR Intensity", L"SSR")
	PROPERTY(_float, m_fSSRMaxDistance, L"SSR Max Distance", L"SSR")
	PROPERTY(_float, m_fSSRThickness, L"SSR Thickness", L"SSR")

	PROPERTY(_bool, m_bDoFEnable, L"DoF Enable", L"DoF")
	PROPERTY(_float, m_fFocusDist, L"DoF Focus Dist", L"DoF")
	PROPERTY(_float, m_fAperture, L"DoF Aperture", L"DoF")
	PROPERTY(_float, m_fDoFMaxCoC, L"DoF Max Blur", L"DoF")
	PROPERTY(_bool, m_bDoFAutoFocus, L"DoF Auto Focus", L"DoF")

	PROPERTY(_float, m_fExposure, L"Exposure", L"Tone Mapping")
	PROPERTY(_float, m_fToneMapMode, L"ToneMap Mode", L"Tone Mapping")
	PROPERTY(_float, m_fAmbientSaturation, L"Ambient Saturation", L"Ambient")

	PROPERTY(_bool, m_bFogEnable, L"Fog Enable", L"Fog")
	PROPERTY(_float, m_fFogDensity, L"Fog Density", L"Fog")
	PROPERTY(_float3, m_vFogColor, L"Fog Color", L"Fog")
	PROPERTY(_float, m_fFogFar, L"Fog Far", L"Fog")
	PROPERTY(_float, m_fFogStart, L"Fog Start", L"Fog")
	PROPERTY(_float, m_fFogStartRange, L"Fog Start Range", L"Fog")
	PROPERTY(_float, m_fFogHeightFalloff, L"Fog Height Falloff", L"Fog")
	PROPERTY(_float, m_fFogBaseHeight, L"Fog Base Height", L"Fog")
	PROPERTY(_float, m_fFogAnisotropy, L"Fog Anisotropy", L"Fog")
	PROPERTY(_float, m_fFogAmbient, L"Fog Ambient", L"Fog")
	PROPERTY(_float, m_fFogShadowStrength, L"Fog Shadow Strength", L"Fog")
	PROPERTY(_float, m_fFogLightIntensity, L"Fog Light Intensity", L"Fog")

	PROPERTY(_float3, m_vAtmosColor, L"Atmos Color", L"Atmosphere")
	PROPERTY(_float, m_fAtmosStart, L"Atmos Start", L"Atmosphere")
	PROPERTY(_float, m_fAtmosEnd, L"Atmos End", L"Atmosphere")
	PROPERTY(_float, m_fAtmosStrength, L"Atmos Strength", L"Atmosphere")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvTrigger_RenderGlobals";

private:
	CEnvTrigger_RenderGlobals(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvTrigger_RenderGlobals(const CEnvTrigger_RenderGlobals& Prototype);
	virtual ~CEnvTrigger_RenderGlobals() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	void Apply_RenderGlobals();

private:
	virtual void OnTriggerEnter(CCollider* pOther) override;
	virtual void OnTriggerStay(CCollider* pOther) override;
	virtual void OnTriggerExit(CCollider* pOther) override;

	_bool Is_TriggerActivator(CCollider* pOther) const;

public:
	static CEnvTrigger_RenderGlobals* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END