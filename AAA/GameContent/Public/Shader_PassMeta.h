#pragma once
#include "GameContent_const.h"

NS_BEGIN(Client)

enum class MAP_PASS : _uint { SHADOW = 0, WHITE, DEFAULT, OVERLAY, TOP, _COUNT };

namespace ShaderPass
{
	namespace Map
	{
		inline constexpr _uint Default = 0;
		inline constexpr _uint Overlay = 1;
		inline constexpr _uint White = 2;
		inline constexpr _uint Shadow = 3;
	}

	namespace NonAnimPBR
	{
		inline constexpr _uint Default = 0;
		inline constexpr _uint Diffuse = 1;
		inline constexpr _uint Shadow = 2;
		inline constexpr _uint White = 3;
		inline constexpr _uint Dither = 4;
		inline constexpr _uint DIFF = 5;
		inline constexpr _uint DMN = 6;	// Diffuse MRA Normal
		inline constexpr _uint UKWN = 7;
		inline constexpr _uint UMN = 8;	// Unkwown MRA Normal
	}

	namespace EnvInst
	{
		inline constexpr _uint SHADOW = 0;
		inline constexpr _uint WHITE = 1;
		inline constexpr _uint DIFF = 2;
		inline constexpr _uint DMN = 3;	// Diffuse MRA Normal
		inline constexpr _uint UKWN = 4;
		inline constexpr _uint UMN = 5;	// Unkwown MRA Normal
	}

	namespace EnvInstFlags
	{
		inline constexpr _uint Dither = 1u << 0;
	}
}

struct ENV_SHADER_PASS_META
{
	_int			iEnvPass;
	_uint			iNonAnimPass;
	const _char*	szName;
	_uint			iRequiredTextureMask;
};

enum ENV_PASS_TEX_MASK : _uint
{
	ENV_PASS_TEX_DIFFUSE	= 1u << 0,
	ENV_PASS_TEX_NORMAL		= 1u << 1,
	ENV_PASS_TEX_MRA		= 1u << 2,
	ENV_PASS_TEX_UNKNOWN	= 1u << 3,
};

inline constexpr ENV_SHADER_PASS_META g_EnvShaderPassMetas[] =
{
	{ -1,                         ShaderPass::NonAnimPBR::DIFF,  "Default", ENV_PASS_TEX_DIFFUSE },
	{ ShaderPass::EnvInst::WHITE, ShaderPass::NonAnimPBR::White, "WHITE",   0 },
	{ ShaderPass::EnvInst::DIFF,  ShaderPass::NonAnimPBR::DIFF,  "DIFF",    ENV_PASS_TEX_DIFFUSE },
	{ ShaderPass::EnvInst::DMN,   ShaderPass::NonAnimPBR::DMN,   "DMN",     ENV_PASS_TEX_DIFFUSE | ENV_PASS_TEX_MRA | ENV_PASS_TEX_NORMAL },
	{ ShaderPass::EnvInst::UKWN,  ShaderPass::NonAnimPBR::UKWN,  "UKWN",    ENV_PASS_TEX_UNKNOWN },
	{ ShaderPass::EnvInst::UMN,   ShaderPass::NonAnimPBR::UMN,   "UMN",     ENV_PASS_TEX_UNKNOWN | ENV_PASS_TEX_MRA | ENV_PASS_TEX_NORMAL },
};

inline const ENV_SHADER_PASS_META* Find_EnvShaderPassMeta(_int iEnvPass)
{
	for (const auto& Meta : g_EnvShaderPassMetas)
	{
		if (Meta.iEnvPass == iEnvPass)
			return &Meta;
	}

	return &g_EnvShaderPassMetas[0];
}

inline _int Get_EnvShaderPassComboIndex(_int iEnvPass)
{
	for (_uint i = 0; i < _countof(g_EnvShaderPassMetas); ++i)
	{
		if (g_EnvShaderPassMetas[i].iEnvPass == iEnvPass)
			return static_cast<_int>(i);
	}

	return 0;
}

inline _int Get_EnvShaderPassFromComboIndex(_int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(_countof(g_EnvShaderPassMetas)))
		return -1;

	return g_EnvShaderPassMetas[iIndex].iEnvPass;
}

NS_END