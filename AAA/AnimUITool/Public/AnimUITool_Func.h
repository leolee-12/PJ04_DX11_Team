#pragma once
#ifndef AnimUITool_Function_h__
#define AnimUITool_Function_h__

#include <vector>
#include <string>
#include "AnimUITool_Struct.h"

namespace AnimUITool
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


    // Engine 쪽 태그는 대부분 _wstring, wstring -> ImGui의 기본 API는 const char*를 필요로 함.
    inline string ToUtf8(const _wstring& str)
    {
        if (str.empty())
            return {};

        // 변환 결과가 몇 바이트 필요한지 계산
        const int iSize = WideCharToMultiByte(
            CP_UTF8, 0,
            str.data(), static_cast<int>(str.size()),
            nullptr, 0,
            nullptr, nullptr);

        if (iSize <= 0)
            return {};

        // 필요한 크기만큼 문자열 버퍼 생성
        std::string strResult(iSize, '\0');

        // 변환해서 실제로 채우기
        WideCharToMultiByte(
            CP_UTF8, 0,
            str.data(), static_cast<int>(str.size()),
            strResult.data(), iSize,
            nullptr, nullptr);

        return strResult;
    }
}

#endif // AnimUITool_Function_h__