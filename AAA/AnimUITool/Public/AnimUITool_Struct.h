#pragma once
#ifndef AnimUITool_Struct_h__
#define AnimUITool_Struct_h__

namespace AnimUITool
{
    typedef struct tagLogDesc
    {
        LOG_LEVEL    eLevel = { LOG_LEVEL::INFO };
        std::string  strMessage = {};
    } LOG_DESC;
}

#endif // AnimUITool_Struct_h__