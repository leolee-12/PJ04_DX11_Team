#pragma once

#include "Engine_Defines.h"
#include "GameContent_Defines.h"
#include "GameObject.h"

#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

NS_BEGIN(Client)

enum class MAP_SECTION_TYPE
{
	DEFAULT,
	BUILDING,
	GROUND,
	ROCK,
	TRANSPARENT,
	WATER,
	EFFECT,
	UNKNOWN,
	END
};

inline constexpr _uint MAP_SECTION_TYPE_COUNT = static_cast<_uint>(MAP_SECTION_TYPE::END);

struct MAP_SECTION_DESC : public CGameObject::GAMEOBJECT_DESC
{
	_wstring			strSectionName;
	_wstring			strModelProtoTag;
	_uint				iModelProtoLevel = {};
	MAP_SECTION_TYPE	eSectionType = { MAP_SECTION_TYPE::UNKNOWN };
	RENDERID			eRenderID = { RENDERID::NONBLEND };
	_bool				bCastShadow = { false };
	_bool				bEnableCulling = { true };
	_bool				bRenderable = { true };
};

struct MAP_STAGE_DESC : public CGameObject::GAMEOBJECT_DESC
{
	_wstring					strStageName;
	_uint						iSectionProtoLevel = {};
	vector<MAP_SECTION_DESC>	SectionDescs;
};

struct MAP_SECTION_PROFILE
{
	double	dRenderCpuMs = {};
	_uint	iEstimatedDrawCalls = {};
};

struct MAP_STAGE_PROFILE
{
	_uint	iFrameIndex = {};
	_uint	iTotalSections = {};
	_uint	iVisibleSections = {};
	_uint	iCulledSections = {};
	_uint	iSubmittedNonBlend = {};
	_uint	iSubmittedBlend = {};
	_uint	iSubmittedShadow = {};
	_uint	iSectionTypeCount[MAP_SECTION_TYPE_COUNT] = {};
	_uint	iEstimatedDrawCalls = {};
	double	dStageLateUpdateCpuMs = {};
	double	dCullingCpuMs = {};
	double	dSectionRenderCpuMs = {};
};

NS_END
