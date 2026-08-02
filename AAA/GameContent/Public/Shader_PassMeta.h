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

	EDGEDITHER,   // 10 - 경계에 Dither 적용

	_COUNT
};

inline constexpr MAP_PASS MAP_DEFAULT_PASS = MAP_PASS::WHITE;

struct MAP_SHADER_PASS_META
{
	MAP_PASS		ePass;
	const _char*	szName;
};

inline constexpr MAP_SHADER_PASS_META g_MapShaderPassMetas[] =
{
	  { MAP_PASS::SHADOW,		"Shadow" },
	  { MAP_PASS::WHITE,		"White" },
	  { MAP_PASS::DIFF,			"DIFF" },
	  { MAP_PASS::DN,			"DN" },
	  { MAP_PASS::DMN,			"DMN" },
	  { MAP_PASS::DMNU,			"DMNU" },
	  { MAP_PASS::TOP,			"FRONT" },
	  { MAP_PASS::MASK,			"MASK" },
	  { MAP_PASS::UKWN,			"UKWN" },
	  { MAP_PASS::DISCARD,		"DISCARD" },
	  { MAP_PASS::EDGEDITHER,	"EDGEDITHER" },
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
	inline constexpr _uint NearDither = 1u << 1;
	inline constexpr _uint LavaEdgeDither = 1u << 2;
	inline constexpr _uint EmissiveMono = 1u << 3;
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
	BLEND_DCUT_UMN,
	BLEND_DMN,			// 17 - BLEND_HDR에서 렌더링하는 포워드 반투명 패스
	BLEND_UKWN_LIGHT,	// 18 - UNKNOWN 밝기를 마스크로 사용하는 빛줄기 패스
	BLEND_UKWN2_LIGHT,	// 19 - 두 UNKNOWN 텍스처를 조합하는 빛줄기 패스
	UKWN2_SAND_OPAQUE,	// 20
	BLEND_UKWN_BARRIER,	// 21 - 배틀 경계: 카메라 근접 페이드 + 상승 물방울
	LAVA_SURFACE,		// 22 - MRA.G 크러스트 마스크 + flow 왜곡 용암 표면
	CUT_CROSSFADE,		// 23 - UNKNOWN(Src) <-> ExtraR(Dst) 크로스페이드. 엔딩 크레딧 액자 사진
	DMN_EMISSIVE,		// 24 - DMN + EMISSIVE 텍스처를 GBuffer Emissive에 기록

	COUNT
};

struct WORLD_SHADER_PASS_META
{
	WORLD_PASS		ePass;
	const _char*	szName;
};

inline constexpr WORLD_SHADER_PASS_META g_WorldShaderPassMetas[] =
{
	{ WORLD_PASS::DEFAULT,				"Default" },
	{ WORLD_PASS::WHITE,				"WHITE" },
	{ WORLD_PASS::DIFF,					"DIFF" },
	{ WORLD_PASS::DMN,					"DMN" },
	{ WORLD_PASS::UKWN,					"UKWN" },
	{ WORLD_PASS::UMN,					"UMN" },
	{ WORLD_PASS::DMNU,					"DMNU" },
	{ WORLD_PASS::TREESHADOW,			"TREESHADOW" },
	{ WORLD_PASS::DCUT_COLOR,			"DCUT_COLOR" },
	{ WORLD_PASS::COLOR,				"COLOR" },
	{ WORLD_PASS::DISCARD,				"DISCARD" },
	{ WORLD_PASS::COLOR_CONST_MRA,		"COLOR_CONST_MRA" },
	{ WORLD_PASS::ARROWBOARD_OPAQUE,	"ARROWBOARD_OPAQUE" },
	{ WORLD_PASS::DMN_OPAQUE,			"DMN_OPAQUE" },
	{ WORLD_PASS::BLEND_UKWN_OVERLAY,	"BLEND_UKWN_OVERLAY" },
	{ WORLD_PASS::BLEND_DCUT_UMN,		"BLEND_DCUT_UMN" },
	{ WORLD_PASS::BLEND_DMN,			"BLEND_DMN" },
	{ WORLD_PASS::BLEND_UKWN_LIGHT,		"BLEND_UKWN_LIGHT" },
	{ WORLD_PASS::BLEND_UKWN2_LIGHT,	"BLEND_UKWN2_LIGHT" },
	{ WORLD_PASS::UKWN2_SAND_OPAQUE,	"UKWN2_SAND_OPAQUE" },
	{ WORLD_PASS::BLEND_UKWN_BARRIER,	"BLEND_UKWN_BARRIER" },
	{ WORLD_PASS::LAVA_SURFACE,			"LAVA_SURFACE" },
	{ WORLD_PASS::CUT_CROSSFADE,		"CUT_CROSSFADE" },
	{ WORLD_PASS::DMN_EMISSIVE,			"DMN_EMISSIVE" },
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
	case WORLD_PASS::BLEND_DCUT_UMN:
	case WORLD_PASS::BLEND_DMN:
	case WORLD_PASS::BLEND_UKWN_LIGHT:
	case WORLD_PASS::BLEND_UKWN2_LIGHT:
	case WORLD_PASS::BLEND_UKWN_BARRIER:
		return true;

	default:
		return false;
	}
}

inline SHADOW_ALPHA_SOURCE Resolve_WorldShadowAlphaSource(WORLD_PASS ePass)
{
	switch (ePass)
	{
	case WORLD_PASS::DEFAULT:
	case WORLD_PASS::DIFF:
	case WORLD_PASS::DMN:
	case WORLD_PASS::DCUT_COLOR:
	case WORLD_PASS::DMN_EMISSIVE:
		return SHADOW_ALPHA_SOURCE::DIFFUSE;

	case WORLD_PASS::DMNU:
		return SHADOW_ALPHA_SOURCE::UNKNOWN_R;

	case WORLD_PASS::UKWN:
	case WORLD_PASS::UMN:
		return SHADOW_ALPHA_SOURCE::UNKNOWN;

	case WORLD_PASS::TREESHADOW:
		return SHADOW_ALPHA_SOURCE::DIFFUSE_R;

	case WORLD_PASS::DMN_OPAQUE:
	case WORLD_PASS::UKWN2_SAND_OPAQUE:
		return SHADOW_ALPHA_SOURCE::NONE;

	case WORLD_PASS::DISCARD:
	case WORLD_PASS::BLEND_UKWN_OVERLAY:
	case WORLD_PASS::BLEND_DCUT_UMN:
	case WORLD_PASS::BLEND_DMN:
	case WORLD_PASS::BLEND_UKWN_LIGHT:
	case WORLD_PASS::BLEND_UKWN2_LIGHT:
	case WORLD_PASS::BLEND_UKWN_BARRIER:
		return SHADOW_ALPHA_SOURCE::DISCARD_ALL;

	default:
		return SHADOW_ALPHA_SOURCE::NONE;
	}
}

inline _bool Uses_WorldExtraRSlot(_int iPass)
{
	switch (static_cast<WORLD_PASS>(iPass))
	{
	case WORLD_PASS::BLEND_UKWN2_LIGHT:
	case WORLD_PASS::UKWN2_SAND_OPAQUE:
	case WORLD_PASS::BLEND_UKWN_BARRIER:
	case WORLD_PASS::CUT_CROSSFADE:
		return true;

	default:
		return false;
	}
}

inline _bool Uses_WorldEmissiveSlot(_int iPass)
{
	switch (static_cast<WORLD_PASS>(iPass))
	{
	case WORLD_PASS::BLEND_DCUT_UMN:
	case WORLD_PASS::DMN_EMISSIVE:
		return true;

	default:
		return false;
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
enum class NONANIM_MESH_PASS : _uint
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
NS_END