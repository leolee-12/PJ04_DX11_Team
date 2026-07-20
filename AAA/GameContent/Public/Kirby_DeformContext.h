#pragma once

#include "GameContent_Defines.h"

NS_BEGIN(Client)

class IDeformable;

struct POST_DEFORM_END_CONTEXT
{
    IDeformable* pDeformable{};
    _float3 vStartPos{};
    _float3 vStartLook{};
    DEFORM_TYPE eDeformType{};
};

NS_END