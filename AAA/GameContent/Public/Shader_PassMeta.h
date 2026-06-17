#pragma once
#include "GameContent_const.h"

NS_BEGIN(Client)

enum class MAP_PASS : _uint
{
	SHADOW = 0,
	WHITE,

	DIFF,
	DN,
	DMN,
	DMNU,

	_COUNT
};

inline constexpr MAP_PASS MAP_DEFAULT_PASS = MAP_PASS::WHITE;

inline _bool Is_ValidMapPassValue(_int iPass)
{
	return 0 <= iPass && iPass < ETOI(MAP_PASS::_COUNT);
}

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

struct MAP_SHADER_PASS_META
{
	MAP_PASS		ePass;
	const _char*	szName;
	_uint			iRequiredTextureMask;
};

struct ENV_SHADER_PASS_META
{
	_int			iEnvPass;
	_uint			iNonAnimPass;
	const _char*	szName;
	_uint			iRequiredTextureMask;
};

enum PASS_TEX_MASK : _uint
{
	PASS_TEX_DIFFUSE	= 1u << 0,
	PASS_TEX_NORMAL		= 1u << 1,
	PASS_TEX_MRA		= 1u << 2,
	PASS_TEX_UNKNOWN	= 1u << 3,
};

inline constexpr MAP_SHADER_PASS_META g_MapShaderPassMetas[] =
{
	{ MAP_PASS::SHADOW,	"Shadow",	0 },
	{ MAP_PASS::WHITE,	"White",	0 },
	{ MAP_PASS::DIFF,	"DIFF",		PASS_TEX_DIFFUSE },
	{ MAP_PASS::DN,		"DN",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL },
	{ MAP_PASS::DMN,	"DMN",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL | PASS_TEX_MRA },
	{ MAP_PASS::DMNU,	"DMNU",		PASS_TEX_DIFFUSE | PASS_TEX_NORMAL | PASS_TEX_MRA | PASS_TEX_UNKNOWN },
};

inline constexpr ENV_SHADER_PASS_META g_EnvShaderPassMetas[] =
{
	{ -1,                         ShaderPass::NonAnimPBR::DIFF,  "Default", PASS_TEX_DIFFUSE },
	{ ShaderPass::EnvInst::WHITE, ShaderPass::NonAnimPBR::White, "WHITE",   0 },
	{ ShaderPass::EnvInst::DIFF,  ShaderPass::NonAnimPBR::DIFF,  "DIFF",    PASS_TEX_DIFFUSE },
	{ ShaderPass::EnvInst::DMN,   ShaderPass::NonAnimPBR::DMN,   "DMN",     PASS_TEX_DIFFUSE | PASS_TEX_MRA | PASS_TEX_NORMAL },
	{ ShaderPass::EnvInst::UKWN,  ShaderPass::NonAnimPBR::UKWN,  "UKWN",    PASS_TEX_UNKNOWN },
	{ ShaderPass::EnvInst::UMN,   ShaderPass::NonAnimPBR::UMN,   "UMN",     PASS_TEX_UNKNOWN | PASS_TEX_MRA | PASS_TEX_NORMAL },
};

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