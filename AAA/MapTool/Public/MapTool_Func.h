#pragma once
#include <vector>
#include <string>
#include "MapTool_Struct.h"

namespace MapTool
{
    inline std::vector<LOG_DESC>& Get_LogBuffer()
    {
        static std::vector<LOG_DESC> s_LogBuffer;   
        return s_LogBuffer;
    }

    inline void Log_Message(LOG_LEVEL eLevel, const std::string& strMsg)
    {
        auto& buf = Get_LogBuffer();
        constexpr size_t MAX_LOG = 2000;            
        if (buf.size() >= MAX_LOG)
            buf.erase(buf.begin(), buf.begin() + (buf.size() - MAX_LOG + 1));
        buf.push_back({ eLevel, strMsg });
    }

    inline void Log_Info(const std::string& strMsg) { Log_Message(LOG_LEVEL::INFO, strMsg); }
    inline void Log_Warning(const std::string& strMsg) { Log_Message(LOG_LEVEL::WARNING, strMsg); }
    inline void Log_Error(const std::string& strMsg) { Log_Message(LOG_LEVEL::ERROR_, strMsg); }
    inline void Clear_Log() { Get_LogBuffer().clear(); }
}