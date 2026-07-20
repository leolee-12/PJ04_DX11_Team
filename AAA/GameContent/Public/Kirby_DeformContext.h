#pragma once

#include "GameContent_Defines.h"

NS_BEGIN(Client)

class IDeformable;
enum class DEFORM_OBJECT_KIND;

struct POST_DEFORM_END_CONTEXT
{
    IDeformable* pDeformable{};
    DEFORM_TYPE eDeformType{};
    DEFORM_OBJECT_KIND eDeformKind{};

    _float3 vStartPos{};
    _float3 vStartLook{};
};

NS_END