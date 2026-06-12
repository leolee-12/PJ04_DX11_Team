#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

enum class UI_EFFECT_PASS : _int
{
    MASKED_TEXTURE = 2,
    MASKED_COLOR = 3,
    MASKED_ADD = 4,
    BRUSH_REVEAL = 5,
    GAUGE_FILL_COLOR = 6,
    GAUGE_FILL_TEXTURE = 7,
    END
};

enum class UI_GAUGE_FILL_DIRECTION : _int
{
    LEFT_TO_RIGHT = 0,
    RIGHT_TO_LEFT = 1,
};

enum class UI_GAUGE_LAYER_TYPE : _uint
{
    BACK,
    HEAL,
    FILL,
    END
};

NS_END
