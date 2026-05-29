#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Client)

enum class RESOURCE_TYPE : _uint
{
    TEXTURE,
    VIBUFFER,
};

struct RESOURCE_DATA
{
    RESOURCE_TYPE   eType;              // 리소스 타입
    wstring         strPrototypeTag;    // 프로토타입 태그
    wstring         strFilePath;        // 파일 경로
    _uint           iNumTextures;       // 텍스처 개수 (텍스처용)
};

NS_END