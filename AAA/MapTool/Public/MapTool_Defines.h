#pragma once
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "Engine_Defines.h"
#include "GameContent_Defines.h"

namespace MapTool
{
	enum class TOOL_LEVEL { STATIC, LOADING, EDIT, END };

	enum class LOG_LEVEL { INFO, WARNING, ERROR_, END };

	inline void Log_Message(LOG_LEVEL eLevel, const std::string& strMsg)
	{
		const char* szPrefix = "[Info]    ";
		switch (eLevel)
		{
		case LOG_LEVEL::WARNING: szPrefix = "[Warning] "; break;
		case LOG_LEVEL::ERROR_:  szPrefix = "[Error]   "; break;
		default: break;
		}
		OutputDebugStringA((std::string(szPrefix) + strMsg + "\n").c_str());
	}

	inline void Log_Info(const std::string& strMsg) { Log_Message(LOG_LEVEL::INFO, strMsg); }
	inline void Log_Warning(const std::string& strMsg) { Log_Message(LOG_LEVEL::WARNING, strMsg); }
	inline void Log_Error(const std::string& strMsg) { Log_Message(LOG_LEVEL::ERROR_, strMsg); }

	static const unsigned int       g_iWinSizeX = { 1600 };
	static const unsigned int       g_iWinSizeY = { 900 };

	static const wchar_t* g_strUIScenePath = { L"../../Resources/UIScene/" };
}

extern HINSTANCE	g_hInstance;
extern HWND			g_hWnd;

using namespace MapTool;