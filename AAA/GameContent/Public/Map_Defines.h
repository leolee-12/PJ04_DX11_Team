#pragma once

#include "Engine_Defines.h"
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

struct MAP_SECTION_DESC : public CGameObject::GAMEOBJECT_DESC
{
	_wstring			strSectionName;
	_wstring			wstrModelProtoTag;
	_wstring			wstrModelPath;
	_uint				iModelProtoLevel = {};
	_bool				bRenderable = { true };
	_bool				bUseCollMesh = { true };
};

struct MAP_STAGE_DESC : public CGameObject::GAMEOBJECT_DESC
{
	_wstring					strStageName;
	_uint						iSectionProtoLevel = {};
	vector<MAP_SECTION_DESC>	SectionDescs;
};

NS_END
