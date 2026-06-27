#pragma once
#include "GameContent_const.h"

NS_BEGIN(Client)

#pragma region Map Pass
enum MAP_LAYER_EX_GROUP : _uint { MAIN, R, G, B, A, GROUP_COUNT };

enum MAP_LAYER_EX_ENTRY : _uint
{
	LAYER_EX_DIFF = 0,
	LAYER_EX_MRA,
	LAYER_EX_NORM,
	LAYER_EX_UKWN,
	LAYER_EX_ENTRY_COUNT
};

enum TEX_KIND : _uint
{
	DIFF = 1u << 0,
	NORM = 1u << 1,
	MRA = 1u << 2,
	UKWN = 1u << 3,

	TEX_KIND_COUNT
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
	const _char*	szName;
	_uint			iRequiredTextureMask;
};

inline constexpr MAP_SHADER_PASS_META g_MapShaderPassMetas[] =
{
	{ MAP_PASS::SHADOW,		"Shadow",	0 },
	{ MAP_PASS::WHITE,		"White",	0 },
	{ MAP_PASS::DIFF,		"DIFF",		DIFF },
	{ MAP_PASS::DN,			"DN",		DIFF | NORM },
	{ MAP_PASS::DMN,		"DMN",		DIFF | NORM | MRA },
	{ MAP_PASS::DMNU,		"DMNU",		DIFF | NORM | MRA | UKWN },
	{ MAP_PASS::TOP,		"TOP",		DIFF | NORM | MRA },
	{ MAP_PASS::MASK,		"MASK",		DIFF | NORM | MRA },
	{ MAP_PASS::UKWN,		"UKWN",		UKWN },
	{ MAP_PASS::DISCARD,	"DISCARD",	0 },
};

static const char* kLayerExTextureNames[MESH_LAYER_EX_GROUP_COUNT][MESH_LAYER_EX_ENTRY_COUNT] =
{
	  { "g_TexDiff_Main", "g_TexMRA_Main", "g_TexNorm_Main", "g_TexUkwn_Main" },
	  { "g_TexDiff_R", "g_TexMRA_R", "g_TexNorm_R", "g_TexUkwn_R" },
	  { "g_TexDiff_G", "g_TexMRA_G", "g_TexNorm_G", "g_TexUkwn_G" },
	  { "g_TexDiff_B", "g_TexMRA_B", "g_TexNorm_B", "g_TexUkwn_B" },
	  { "g_TexDiff_A", "g_TexMRA_A", "g_TexNorm_A", "g_TexUkwn_A" }
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

static DEFAULT_TEXTURE GetLayerExDefaultTexture(_uint iEntry)
{
	switch (iEntry)
	{
	case LAYER_EX_DIFF:	return DEFAULT_TEXTURE::MAGENTA;
	case LAYER_EX_MRA:	return DEFAULT_TEXTURE::MRA;
	case LAYER_EX_NORM:	return DEFAULT_TEXTURE::FLAT_NORMAL;
	
	case LAYER_EX_UKWN:
	default:
		return DEFAULT_TEXTURE::BLACK;
	}
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
	COLORMRADITHER,
	DECAL,

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
		inline constexpr _uint COLORMRADITHER = 14;
		inline constexpr _uint DECAL = 15;
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
	{ ENV_PASS::DEFAULT,		ShaderPass::NonAnimPBR::DMN,			"Default",			DIFF | MRA | NORM },
	{ ENV_PASS::WHITE,			ShaderPass::NonAnimPBR::White,			"WHITE",			0 },
	{ ENV_PASS::DIFF,			ShaderPass::NonAnimPBR::DIFF,			"DIFF",				DIFF },
	{ ENV_PASS::DMN,			ShaderPass::NonAnimPBR::DMN,			"DMN",				DIFF | MRA | NORM },
	{ ENV_PASS::UKWN,			ShaderPass::NonAnimPBR::UKWN,			"UKWN",				UKWN },
	{ ENV_PASS::UMN,			ShaderPass::NonAnimPBR::UMN,			"UMN",				UKWN | MRA | NORM },

	{ ENV_PASS::DMNU,			ShaderPass::NonAnimPBR::DMNU,			"DMN",				DIFF | MRA | NORM | UKWN },
	{ ENV_PASS::TREESHADOW,		ShaderPass::NonAnimPBR::TREESHADOW,		"TREESHADOW",		UKWN },
	{ ENV_PASS::GRASS_FUR,		ShaderPass::NonAnimPBR::GRASS_FUR,		"GRASS_FUR",		UKWN | MRA | NORM },
	{ ENV_PASS::MN,				ShaderPass::NonAnimPBR::MN,				"MN",				MRA | NORM },
	{ ENV_PASS::DISCARD,		ShaderPass::NonAnimPBR::DISCARD,		"DISCARD",			0 },
	{ ENV_PASS::COLORMRADITHER,	ShaderPass::NonAnimPBR::COLORMRADITHER,	"COLORMRADITHER",	0 },
	{ ENV_PASS::DECAL,			ShaderPass::NonAnimPBR::DECAL,			"DECAL",			0 },
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