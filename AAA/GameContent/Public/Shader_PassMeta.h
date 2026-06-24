#pragma once
#include "GameContent_const.h"

NS_BEGIN(Client)

#pragma region Map Pass
enum PASS_TEX_MASK : _uint
{
	PASS_TEX_DIFFUSE = 1u << 0,
	PASS_TEX_NORMAL = 1u << 1,
	PASS_TEX_MRA = 1u << 2,
	PASS_TEX_UNKNOWN = 1u << 3,
};

enum class MAP_PASS : _uint
{
	SHADOW = 0,
	WHITE,

	DIFF,
	DN,
	DMN,
	DMNU,

	TOP,
	MASK,

	UKWN,
	DISCARD,

	_COUNT
};

inline constexpr MAP_PASS MAP_DEFAULT_PASS = MAP_PASS::WHITE;

struct MAP_SHADER_PASS_META
{
	MAP_PASS		ePass;
	const _char* szName;
	_uint			iRequiredTextureMask;
};

inline constexpr MAP_SHADER_PASS_META g_MapShaderPassMetas[] =
{
	{ MAP_PASS::SHADOW,		"Shadow",	0 },
	{ MAP_PASS::WHITE,		"White",	0 },
	{ MAP_PASS::DIFF,		"DIFF",		PASS_TEX_DIFFUSE },
	{ MAP_PASS::DN,			"DN",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL },
	{ MAP_PASS::DMN,		"DMN",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL | PASS_TEX_MRA },
	{ MAP_PASS::DMNU,		"DMNU",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL | PASS_TEX_MRA | PASS_TEX_UNKNOWN },
	{ MAP_PASS::TOP,		"TOP",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL | PASS_TEX_MRA },
	{ MAP_PASS::MASK,		"MASK",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL | PASS_TEX_MRA },
	{ MAP_PASS::UKWN,		"UKWN",		PASS_TEX_UNKNOWN },
	{ MAP_PASS::DISCARD,	"DISCARD",	0 },
};

inline _bool Is_ValidMapPassValue(_int iPass)
{
	return 0 <= iPass && iPass < ETOI(MAP_PASS::_COUNT);
}

inline const MAP_SHADER_PASS_META* Find_MapShaderPassMeta(_int iPass)
{
	for (const auto& Meta : g_MapShaderPassMetas)
	{
		if (ETOI(Meta.ePass) == iPass)
			return &Meta;
	}

	return &g_MapShaderPassMetas[static_cast<_uint>(MAP_DEFAULT_PASS)];
}

inline _int Get_MapShaderPassComboIndex(_int iPass)
{
	for (_uint i = 0; i < _countof(g_MapShaderPassMetas); ++i)
	{
		if (ETOI(g_MapShaderPassMetas[i].ePass) == iPass)
			return static_cast<_int>(i);
	}

	return ETOI(MAP_DEFAULT_PASS);
}

inline _int Get_MapShaderPassFromComboIndex(_int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(_countof(g_MapShaderPassMetas)))
		return ETOI(MAP_DEFAULT_PASS);

	return ETOI(g_MapShaderPassMetas[iIndex].ePass);
}
#pragma endregion



#pragma region Env Pass
enum class ENV_PASS : _int
{
	DEFAULT = -1,
	SHADOW,
	WHITE,
	DIFF,
	DMN,
	UKWN,
	UMN,

	DMNU,
	TREESHADOW,
	GRASS_FUR,
	MN,
	DISCARD,

	_COUNT
};

namespace ShaderPass
{
	namespace NonAnimPBR
	{
		inline constexpr _uint Default = 0;
		inline constexpr _uint Diffuse = 1;
		inline constexpr _uint Shadow = 2;
		inline constexpr _uint White = 3;
		inline constexpr _uint Dither = 4;
		inline constexpr _uint DIFF = 5;
		inline constexpr _uint DMN = 6;
		inline constexpr _uint UKWN = 7;
		inline constexpr _uint UMN = 8;

		inline constexpr _uint DMNU = 9;
		inline constexpr _uint TREESHADOW = 10;
		inline constexpr _uint GRASS_FUR = 11;
		inline constexpr _uint MN = 12;
		inline constexpr _uint DISCARD = 13;
	}

	namespace EnvInstFlags
	{
		inline constexpr _uint Dither = 1u << 0;
	}
}

struct ENV_SHADER_PASS_META
{
	ENV_PASS		ePass;
	_uint			iNonAnimPass;
	const _char*	szName;
	_uint			iRequiredTextureMask;
};

inline constexpr ENV_SHADER_PASS_META g_EnvShaderPassMetas[] =
{
	{ ENV_PASS::DEFAULT,	ShaderPass::NonAnimPBR::DMN,		"Default",		PASS_TEX_DIFFUSE | PASS_TEX_MRA | PASS_TEX_NORMAL },
	{ ENV_PASS::WHITE,		ShaderPass::NonAnimPBR::White,		"WHITE",		0 },
	{ ENV_PASS::DIFF,		ShaderPass::NonAnimPBR::DIFF,		"DIFF",			PASS_TEX_DIFFUSE },
	{ ENV_PASS::DMN,		ShaderPass::NonAnimPBR::DMN,		"DMN",			PASS_TEX_DIFFUSE | PASS_TEX_MRA | PASS_TEX_NORMAL },
	{ ENV_PASS::UKWN,		ShaderPass::NonAnimPBR::UKWN,		"UKWN",			PASS_TEX_UNKNOWN },
	{ ENV_PASS::UMN,		ShaderPass::NonAnimPBR::UMN,		"UMN",			PASS_TEX_UNKNOWN | PASS_TEX_MRA | PASS_TEX_NORMAL },

	{ ENV_PASS::DMNU,		ShaderPass::NonAnimPBR::DMNU,		"DMN",			PASS_TEX_DIFFUSE | PASS_TEX_MRA | PASS_TEX_NORMAL | PASS_TEX_UNKNOWN },
	{ ENV_PASS::TREESHADOW,	ShaderPass::NonAnimPBR::TREESHADOW,	"TREESHADOW",	PASS_TEX_UNKNOWN },
	{ ENV_PASS::GRASS_FUR,	ShaderPass::NonAnimPBR::GRASS_FUR,	"GRASS_FUR",	PASS_TEX_UNKNOWN | PASS_TEX_MRA | PASS_TEX_NORMAL },
	{ ENV_PASS::MN,			ShaderPass::NonAnimPBR::MN,			"MN",			PASS_TEX_MRA | PASS_TEX_NORMAL },
	{ ENV_PASS::DISCARD,	ShaderPass::NonAnimPBR::DISCARD,	"DISCARD",		0 },
};

enum class ENV_SHADOW_ALPHA_SOURCE : _uint
{
	NONE = 0u,
	DIFFUSE = 1u,
	UNKNOWN = 2u,
	DISCARD_ALL = 3u,
};

inline ENV_SHADOW_ALPHA_SOURCE Resolve_EnvShadowAlphaSource(ENV_PASS ePass)
{
	switch (ePass)
	{
	case ENV_PASS::DEFAULT:
	case ENV_PASS::DIFF:
	case ENV_PASS::DMN:
	case ENV_PASS::DMNU:
	case ENV_PASS::TREESHADOW:
	case ENV_PASS::GRASS_FUR:
		return ENV_SHADOW_ALPHA_SOURCE::DIFFUSE;

	case ENV_PASS::UKWN:
	case ENV_PASS::UMN:
		return ENV_SHADOW_ALPHA_SOURCE::UNKNOWN;

	case ENV_PASS::DISCARD:
		return ENV_SHADOW_ALPHA_SOURCE::DISCARD_ALL;

	case ENV_PASS::WHITE:
	case ENV_PASS::MN:
	case ENV_PASS::SHADOW:
	default:
		return ENV_SHADOW_ALPHA_SOURCE::NONE;
	}
}

inline const ENV_SHADER_PASS_META* Find_EnvShaderPassMeta(_int iPass)
{
	for (const auto& Meta : g_EnvShaderPassMetas)
	{
		if (ETOI(Meta.ePass) == iPass)
			return &Meta;
	}

	return &g_EnvShaderPassMetas[0];
}

inline _int Get_EnvShaderPassComboIndex(_int iPass)
{
	for (_uint i = 0; i < _countof(g_EnvShaderPassMetas); ++i)
	{
		if (ETOI(g_EnvShaderPassMetas[i].ePass) == iPass)
			return static_cast<_int>(i);
	}

	return 0;
}

inline _int Get_EnvShaderPassFromComboIndex(_int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(_countof(g_EnvShaderPassMetas)))
		return ETOI(ENV_PASS::DEFAULT);

	return ETOI(g_EnvShaderPassMetas[iIndex].ePass);
}
#pragma endregion

NS_END