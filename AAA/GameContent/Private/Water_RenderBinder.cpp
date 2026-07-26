#include "Water_RenderBinder.h"

#include "Math_Utils.h"
#include "Shader.h"

#include <cmath>

namespace
{
	constexpr _double WATER_GAME_TIME_PERIOD = { 60000.0 };
	constexpr _float MIN_NORMAL_TILING = { 0.0001f };
	constexpr _float MIN_FOAM_NOISE_TILING = { 0.0001f };
	constexpr _float MIN_CAUSTIC_TILING = { 0.0001f };
	constexpr _float MAX_WATER_MASK_BLUR = { 8.f };
}

void Client::Sanitize_WaterRenderDesc(WATER_RENDER_DESC* pDesc)
{
	if (nullptr == pDesc)
		return;

	const WATER_RENDER_DESC Default{};

	pDesc->vShallowColor = MathUtils::Sanitize_FiniteFloat4(pDesc->vShallowColor, Default.vShallowColor);
	pDesc->vDeepColor = MathUtils::Sanitize_FiniteFloat4(pDesc->vDeepColor, Default.vDeepColor);
	pDesc->fShallowColorStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fShallowColorStrength, Default.fShallowColorStrength, 0.f, 1.f);
	pDesc->fOpacity = MathUtils::Sanitize_ClampedFloat(pDesc->fOpacity, Default.fOpacity, 0.f, 1.f);
	pDesc->fDepthFadeDistance = MathUtils::Sanitize_MinimumFloat(pDesc->fDepthFadeDistance, Default.fDepthFadeDistance, 0.001f);

	pDesc->vNormalTiling0.x = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vNormalTiling0.x, Default.vNormalTiling0.x, MIN_NORMAL_TILING);
	pDesc->vNormalTiling0.y = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vNormalTiling0.y, Default.vNormalTiling0.y, MIN_NORMAL_TILING);
	pDesc->vNormalTiling1.x = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vNormalTiling1.x, Default.vNormalTiling1.x, MIN_NORMAL_TILING);
	pDesc->vNormalTiling1.y = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vNormalTiling1.y, Default.vNormalTiling1.y, MIN_NORMAL_TILING);

	pDesc->vNormalSpeed0 = MathUtils::Sanitize_FiniteFloat2(pDesc->vNormalSpeed0, Default.vNormalSpeed0);
	pDesc->vNormalSpeed1 = MathUtils::Sanitize_FiniteFloat2(pDesc->vNormalSpeed1, Default.vNormalSpeed1);
	pDesc->fNormalStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fNormalStrength, Default.fNormalStrength, 0.f, 4.f);
	pDesc->fNormalWarpStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fNormalWarpStrength, Default.fNormalWarpStrength, 0.f, 1.f);
	pDesc->fNormalSwayStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fNormalSwayStrength, Default.fNormalSwayStrength, -1.f, 1.f);

	pDesc->fFresnelPower = MathUtils::Sanitize_ClampedFloat(pDesc->fFresnelPower, Default.fFresnelPower, 0.1f, 16.f);
	pDesc->fReflectionStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fReflectionStrength, Default.fReflectionStrength, 0.f, 4.f);
	pDesc->fRefractionStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fRefractionStrength, Default.fRefractionStrength, 0.f, 0.1f);

	pDesc->fLightReceiveStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fLightReceiveStrength, Default.fLightReceiveStrength, 0.f, 1.f);
	pDesc->fSpecularPower = MathUtils::Sanitize_ClampedFloat(pDesc->fSpecularPower, Default.fSpecularPower, 1.f, 256.f);
	pDesc->fSpecularStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fSpecularStrength, Default.fSpecularStrength, 0.f, 8.f);
	pDesc->fSpecularScatter = MathUtils::Sanitize_ClampedFloat(pDesc->fSpecularScatter, Default.fSpecularScatter, 0.f, 8.f);

	pDesc->fFoamWidth = MathUtils::Sanitize_ClampedFloat(pDesc->fFoamWidth, Default.fFoamWidth, 0.f, 5.f);
	pDesc->fFoamStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fFoamStrength, Default.fFoamStrength, 0.f, 4.f);
	pDesc->vFoamNoiseTiling.x = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vFoamNoiseTiling.x, Default.vFoamNoiseTiling.x, MIN_FOAM_NOISE_TILING);
	pDesc->vFoamNoiseTiling.y = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vFoamNoiseTiling.y, Default.vFoamNoiseTiling.y, MIN_FOAM_NOISE_TILING);
	pDesc->vFoamNoiseSpeed = MathUtils::Sanitize_FiniteFloat2(pDesc->vFoamNoiseSpeed, Default.vFoamNoiseSpeed);
	pDesc->fFoamNoiseStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fFoamNoiseStrength, Default.fFoamNoiseStrength, 0.f, 1.f);
	pDesc->fFoamBlur = MathUtils::Sanitize_ClampedFloat(pDesc->fFoamBlur, Default.fFoamBlur, 0.f, MAX_WATER_MASK_BLUR);

	pDesc->vCausticTiling.x = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vCausticTiling.x, Default.vCausticTiling.x, MIN_CAUSTIC_TILING);
	pDesc->vCausticTiling.y = MathUtils::Sanitize_MinimumAbsoluteFloat(pDesc->vCausticTiling.y, Default.vCausticTiling.y, MIN_CAUSTIC_TILING);
	pDesc->vCausticSpeed = MathUtils::Sanitize_FiniteFloat2(pDesc->vCausticSpeed, Default.vCausticSpeed);
	pDesc->fCausticStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fCausticStrength, Default.fCausticStrength, 0.f, 4.f);
	pDesc->fCausticNoiseStrength = MathUtils::Sanitize_ClampedFloat(pDesc->fCausticNoiseStrength, Default.fCausticNoiseStrength, 0.f, 1.f);
	pDesc->fCausticBlur = MathUtils::Sanitize_ClampedFloat(pDesc->fCausticBlur, Default.fCausticBlur, 0.f, MAX_WATER_MASK_BLUR);

	pDesc->fWaveAmplitude = MathUtils::Sanitize_ClampedFloat(pDesc->fWaveAmplitude, Default.fWaveAmplitude, 0.f, 10.f);
	pDesc->fWaveSpeed = MathUtils::Sanitize_ClampedFloat(pDesc->fWaveSpeed, Default.fWaveSpeed, 0.f, 10.f);

	pDesc->fVisibility = MathUtils::Sanitize_ClampedFloat(pDesc->fVisibility, Default.fVisibility, 0.f, 1.f);
}

HRESULT Client::Bind_WaterRenderDesc(CShader* pShader, const WATER_RENDER_DESC& Desc, _double dGameTime)
{
	if (nullptr == pShader)
		return E_INVALIDARG;

	WATER_RENDER_DESC SafeDesc = Desc;
	Sanitize_WaterRenderDesc(&SafeDesc);

	const _double dWrappedGameTime = MathUtils::Wrap_FiniteDouble(dGameTime, WATER_GAME_TIME_PERIOD);
	const _float fGameTime = static_cast<_float>(dWrappedGameTime);

	if (FAILED(pShader->Bind_RawValue("g_vShallowColor", &SafeDesc.vShallowColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vDeepColor", &SafeDesc.vDeepColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fShallowColorStrength", &SafeDesc.fShallowColorStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fOpacity", &SafeDesc.fOpacity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fDepthFadeDistance", &SafeDesc.fDepthFadeDistance, sizeof(_float))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_vNormalTiling0", &SafeDesc.vNormalTiling0, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vNormalSpeed0", &SafeDesc.vNormalSpeed0, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vNormalTiling1", &SafeDesc.vNormalTiling1, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vNormalSpeed1", &SafeDesc.vNormalSpeed1, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fNormalStrength", &SafeDesc.fNormalStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fNormalWarpStrength", &SafeDesc.fNormalWarpStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fNormalSwayStrength", &SafeDesc.fNormalSwayStrength, sizeof(_float))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_fFresnelPower", &SafeDesc.fFresnelPower, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fReflectionStrength", &SafeDesc.fReflectionStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fRefractionStrength", &SafeDesc.fRefractionStrength, sizeof(_float))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_fLightReceiveStrength", &SafeDesc.fLightReceiveStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fSpecularPower", &SafeDesc.fSpecularPower, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fSpecularStrength", &SafeDesc.fSpecularStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fSpecularScatter", &SafeDesc.fSpecularScatter, sizeof(_float))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_fFoamWidth", &SafeDesc.fFoamWidth, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fFoamStrength", &SafeDesc.fFoamStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vFoamNoiseTiling", &SafeDesc.vFoamNoiseTiling, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vFoamNoiseSpeed", &SafeDesc.vFoamNoiseSpeed, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fFoamNoiseStrength", &SafeDesc.fFoamNoiseStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fFoamBlur", &SafeDesc.fFoamBlur, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fVisibility", &SafeDesc.fVisibility, sizeof(_float))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_vCausticTiling", &SafeDesc.vCausticTiling, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vCausticSpeed", &SafeDesc.vCausticSpeed, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fCausticStrength", &SafeDesc.fCausticStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fCausticNoiseStrength", &SafeDesc.fCausticNoiseStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fCausticBlur", &SafeDesc.fCausticBlur, sizeof(_float))))
		return E_FAIL;

	const _float fWavePhase = fGameTime * SafeDesc.fWaveSpeed;
	const _float fWaveOscillation = 0.6f * sinf(fWavePhase) + 0.4f * sinf(fWavePhase * 0.618f + 1.3f);
	const _float fWaveHeight = SafeDesc.fWaveAmplitude * fWaveOscillation;

	if (FAILED(pShader->Bind_RawValue("g_fWaveHeight", &fWaveHeight, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fWaveOscillation", &fWaveOscillation, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fGameTime", &fGameTime, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}