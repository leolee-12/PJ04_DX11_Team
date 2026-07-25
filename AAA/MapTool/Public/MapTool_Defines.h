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

	typedef struct tagLogDesc
	{
		LOG_LEVEL	eLevel = { LOG_LEVEL::INFO };
		std::string	strMessage = {};
	} LOG_DESC;

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

	static const unsigned int       g_iWinSizeX = { 1600 };
	static const unsigned int       g_iWinSizeY = { 900 };

	static const wchar_t* g_strMapModelPath = { L"../../Resources/Maps/" };
	static const wchar_t* g_strUIScenePath = { L"../../Resources/UIScene/" };
}

extern HINSTANCE	g_hInstance;
extern HWND			g_hWnd;

using namespace MapTool;