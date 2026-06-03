#pragma once

namespace MapTool
{
    typedef struct tagLogDesc
    {
        LOG_LEVEL    eLevel = { LOG_LEVEL::INFO };
        std::string  strMessage = {};
    } LOG_DESC;
}