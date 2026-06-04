#pragma once

#include <string>

#include "GameContent_Defines.h"

NS_BEGIN(Client)

enum class GAMECONTENT_LOG_LEVEL
{
	INFO,
	WARNING,
	ERROR_
};

using GAMECONTENT_LOG_SINK = void(*)(GAMECONTENT_LOG_LEVEL eLevel, const char* pMessage);

CLIENT_DLL void Set_GameContentLogSink(GAMECONTENT_LOG_SINK pSink);
CLIENT_DLL void Emit_GameContentLog(GAMECONTENT_LOG_LEVEL eLevel, const char* pMessage);

inline void Log_GameContentInfo(const std::string& strMessage)
{
	Emit_GameContentLog(GAMECONTENT_LOG_LEVEL::INFO, strMessage.c_str());
}

inline void Log_GameContentWarning(const std::string& strMessage)
{
	Emit_GameContentLog(GAMECONTENT_LOG_LEVEL::WARNING, strMessage.c_str());
}

inline void Log_GameContentError(const std::string& strMessage)
{
	Emit_GameContentLog(GAMECONTENT_LOG_LEVEL::ERROR_, strMessage.c_str());
}

NS_END
