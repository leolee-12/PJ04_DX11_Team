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
	{ MAP_PASS::TOP,		"FRONT",	DIFF | NORM | MRA },
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



#pragma region World Pass
// Shader_World_Common.hlsli의 g_iFlags 비트(FLAG_DITHER)와 1:1 대응
namespace WorldShaderFlags
{
	inline constexpr _uint Dither = 1u << 0;
}

enum class SHADOW_ALPHA_SOURCE : _uint
{
	NONE = 0u,
	DIFFUSE = 1u,
	UNKNOWN = 2u,
	DISCARD_ALL = 3u,
	DIFFUSE_R = 4u,
	UNKNOWN_R = 5u,
};

enum class WORLD_PASS : _int
{
	DEFAULT = -1,

	SHADOW = 0,
	WHITE,
	DIFF,
	DMN,
	UKWN,
	UMN,

	DMNU,
	TREESHADOW,
	DCUT_COLOR,
	COLOR,
	DISCARD,
	DECAL,
	COLOR_CONST_MRA,
	ARROWBOARD_OPAQUE,
	DMN_OPAQUE,
	BLEND_UKWN_OVERLAY,
	DCUT_UMN,
	BLEND_DMN, // 17 - BLEND_HDR에서 렌더링하는 포워드 반투명 패스
	BLEND_UKWN_LIGHT, // 18 - UNKNOWN 밝기를 마스크로 사용하는 빛줄기 패스
	BLEND_UKWN2_LIGHT, // 19 - 두 UNKNOWN 텍스처를 조합하는 빛줄기 패스

	COUNT
};

struct WORLD_SHADER_PASS_META
{
	WORLD_PASS		ePass;
	const _char*	szName;
	_uint			iRequiredTextureMask;
};

inline constexpr WORLD_SHADER_PASS_META g_WorldShaderPassMetas[] =
{
	{ WORLD_PASS::DEFAULT,				"Default",				DIFF | MRA | NORM },
	{ WORLD_PASS::WHITE,				"WHITE",				0 },
	{ WORLD_PASS::DIFF,					"DIFF",					DIFF },
	{ WORLD_PASS::DMN,					"DMN",					DIFF | MRA | NORM },
	{ WORLD_PASS::UKWN,					"UKWN",					UKWN },
	{ WORLD_PASS::UMN,					"UMN",					UKWN | MRA | NORM },

	{ WORLD_PASS::DMNU,					"DMNU",					DIFF | MRA | NORM | UKWN },
	{ WORLD_PASS::TREESHADOW,			"TREESHADOW",			DIFF },
	{ WORLD_PASS::DCUT_COLOR,			"DCUT_COLOR",			DIFF | MRA | NORM },
	{ WORLD_PASS::COLOR,				"COLOR",				MRA | NORM },
	{ WORLD_PASS::DISCARD,				"DISCARD",				0 },
	{ WORLD_PASS::COLOR_CONST_MRA,		"COLOR_CONST_MRA",		NORM },
	{ WORLD_PASS::ARROWBOARD_OPAQUE,	"ARROWBOARD_OPAQUE",	DIFF | MRA | NORM },
	{ WORLD_PASS::DMN_OPAQUE,			"DMN_OPAQUE",			DIFF | MRA | NORM },
	{ WORLD_PASS::BLEND_UKWN_OVERLAY,	"BLEND_UKWN_OVERLAY",	UKWN },
	{ WORLD_PASS::DCUT_UMN,				"DCUT_UMN",				DIFF | MRA | NORM | UKWN },
	{ WORLD_PASS::BLEND_DMN,			"BLEND_DMN",			DIFF | MRA | NORM },
	{ WORLD_PASS::BLEND_UKWN_LIGHT,		"BLEND_UKWN_LIGHT",		UKWN },
	{ WORLD_PASS::BLEND_UKWN2_LIGHT,	"BLEND_UKWN2_LIGHT",	UKWN },
};

inline _bool Is_ValidWorldPassValue(_int iPass)
{
	return 0 <= iPass && iPass < ETOI(WORLD_PASS::COUNT);
}

inline _bool Is_WorldBlendPass(_int iPass)
{
	switch (static_cast<WORLD_PASS>(iPass))
	{
	case WORLD_PASS::BLEND_UKWN_OVERLAY:
	case WORLD_PASS::BLEND_DMN:
	case WORLD_PASS::BLEND_UKWN_LIGHT:
	case WORLD_PASS::BLEND_UKWN2_LIGHT:
		return true;

	default:
		return false;
	}
}

inline const WORLD_SHADER_PASS_META* Find_WorldShaderPassMeta(_int iPass)
{
	for (const auto& Meta : g_WorldShaderPassMetas)
	{
		if (ETOI(Meta.ePass) == iPass)
			return &Meta;
	}

	return &g_WorldShaderPassMetas[0];
}

inline SHADOW_ALPHA_SOURCE Resolve_WorldShadowAlphaSource(WORLD_PASS ePass)
{
	switch (ePass)
	{
	case WORLD_PASS::DEFAULT:
	case WORLD_PASS::DIFF:
	case WORLD_PASS::DMN:
	case WORLD_PASS::DCUT_COLOR:
	case WORLD_PASS::DCUT_UMN:
		return SHADOW_ALPHA_SOURCE::DIFFUSE;

	case WORLD_PASS::DMNU:
		return SHADOW_ALPHA_SOURCE::UNKNOWN_R;

	case WORLD_PASS::UKWN:
	case WORLD_PASS::UMN:
		return SHADOW_ALPHA_SOURCE::UNKNOWN;

	case WORLD_PASS::TREESHADOW:
		return SHADOW_ALPHA_SOURCE::DIFFUSE_R;

	case WORLD_PASS::DMN_OPAQUE:
		return SHADOW_ALPHA_SOURCE::NONE;

	case WORLD_PASS::DISCARD:
	case WORLD_PASS::BLEND_UKWN_OVERLAY:
	case WORLD_PASS::BLEND_DMN:
	case WORLD_PASS::BLEND_UKWN_LIGHT:
	case WORLD_PASS::BLEND_UKWN2_LIGHT:
		return SHADOW_ALPHA_SOURCE::DISCARD_ALL;

	default:
		return SHADOW_ALPHA_SOURCE::NONE;
	}
}

inline _int Get_WorldShaderPassComboIndex(_int iPass)
{
	for (_uint i = 0; i < _countof(g_WorldShaderPassMetas); ++i)
	{
		if (ETOI(g_WorldShaderPassMetas[i].ePass) == iPass)
			return static_cast<_int>(i);
	}

	return 0;
}

inline _int Get_WorldShaderPassFromComboIndex(_int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(_countof(g_WorldShaderPassMetas)))
		return ETOI(WORLD_PASS::DEFAULT);

	return ETOI(g_WorldShaderPassMetas[iIndex].ePass);
}
#pragma endregion



#pragma region Anim Mesh Pass
enum class ANIM_MESH_PASS : _uint
{
	DEFAULT = 0,
	NON_EYE,
	TEST,
	CONSTANT_MATERIAL,
	RESERVED_BUSH,
	RESERVED_BOX,
	EYE_WITHOUT_NORMAL,
	SHADOW,
	RESERVED_ARROWBOARD_OPAQUE,
	CAGE,

	COUNT
};
#pragma endregion

#pragma region NonAnim Mesh Pass
//enum class NONANIM_MESH_PASS : _uint
//{
//	DEFAULT = 0,
//	NON_EYE,
//	TEST,
//	CONSTANT_MATERIAL,
//	RESERVED_BUSH,
//	RESERVED_BOX,
//	EYE_WITHOUT_NORMAL,
//	SHADOW,
//	RESERVED_ARROWBOARD_OPAQUE,
//	CAGE,
//
//	COUNT
//};
#pragma endregion
NS_END