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
	_wstring			wstrModelProtoTag;
	_wstring			wstrModelPath;
	_uint				iModelProtoLevel = {};
	MAP_SECTION_TYPE	eSectionType = { MAP_SECTION_TYPE::UNKNOWN };
	RENDERID			eRenderID = { RENDERID::NONBLEND };
	_bool				bEnableCulling = { true };
	_bool				bRenderable = { true };
	_bool				bUseCollMesh = { true };
};

struct MAP_STAGE_DESC : public CGameObject::GAMEOBJECT_DESC
{
	_wstring					strStageName;
	_uint						iSectionProtoLevel = {};
	vector<MAP_SECTION_DESC>	SectionDescs;
};

struct MAP_SECTION_PRESET
{
	const _tchar* pSectionName;
	Client::MAP_SECTION_TYPE eType;
	RENDERID eRenderID;
};

struct MAP_PRESET
{
	const _char* pLabel;
	const _tchar* pStageName;
	const MAP_SECTION_PRESET* pSections;
	_uint iSectionCount;

	const _tchar* const* pEnvJsonPaths;
	_uint iEnvJsonPathCount;
};

NS_END
