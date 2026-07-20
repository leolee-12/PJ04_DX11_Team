#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

enum class WATER_PASS : _uint
{
	SURFACE,
	COUNT
};

struct WATER_RENDER_DESC
{
	// Color / Depth
	_float4 vShallowColor = { 0.15f, 0.55f, 0.72f, 1.f };
	_float4 vDeepColor = { 0.02f, 0.16f, 0.30f, 1.f };
	_float  fOpacity = { 0.65f };
	_float  fDepthFadeDistance = { 5.f };

	// Normal Layer 0
	_float2 vNormalTiling0 = { 0.08f, 0.08f };
	_float2 vNormalSpeed0 = { 0.02f, 0.01f };

	// Normal Layer 1
	_float2 vNormalTiling1 = { 0.14f, 0.14f };
	_float2 vNormalSpeed1 = { -0.012f, 0.018f };
	_float  fNormalStrength = { 1.f };

	// Reflection / Refraction
	_float  fFresnelPower = { 5.f };
	_float  fReflectionStrength = { 0.8f };
	_float  fRefractionStrength = { 0.015f };

	// Specular / HDR
	_float  fSpecularPower = { 64.f };
	_float  fSpecularStrength = { 1.f };

	// Foam
	_float  fFoamWidth = { 0.4f };
	_float  fFoamStrength = { 0.7f };
	_float2 vFoamNoiseTiling = { 0.35f, 0.35f };
	_float2 vFoamNoiseSpeed = { 0.03f, -0.02f };
	_float  fFoamNoiseStrength = { 0.6f };

	// Caustic
	_float2 vCausticTiling = { 0.05f, 0.05f };
	_float2 vCausticSpeed = { 0.015f, 0.012f };
	_float  fCausticStrength = { 0.5f };

	// Runtime only; do not serialize.
	_float  fVisibility = { 1.f };
};

NS_END