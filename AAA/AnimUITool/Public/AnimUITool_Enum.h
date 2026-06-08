#pragma once
#ifndef AnimUITool_Enum_h__
#define AnimUITool_Enum_h__

namespace AnimUITool
{
    enum class TOOL_LEVEL { STATIC, EDIT, END };

    enum class TOOL_MODE { ANIMATION, UI, END };

    enum class LOG_LEVEL { INFO, WARNING, ERROR_, END };

    enum class PREVIEW_SHADER_MODE
    {
        AUTO,
        ANIM_MESH,
        NONANIM_MESH,
        END
    };

    enum class UI_DRAG_MODE
    {
        NONE,
        MOVE,
        RESIZE_TL,
        RESIZE_TR,
        RESIZE_BL,
        RESIZE_BR,
        END
    };

    enum class UI_PART_TYPE { IMAGE, SPRITEANIM, TEXT, END};
}

#endif // AnimUITool_Enum_h__