#pragma once
#include "Engine_Defines.h"
#include "ObjectData.h"

NS_BEGIN(Client)

struct LEVEL_DATA
{
    wstring             strLevelName;   // 레벨 이름
    vector<OBJECT_DATA> vecObjects;     // 오브젝트 목록
};

NS_END