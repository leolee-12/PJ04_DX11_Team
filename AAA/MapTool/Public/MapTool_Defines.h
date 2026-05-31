#pragma once
#include <Windows.h>
#include <process.h>

#include "GameContent_Defines.h"

using namespace Engine;

#include "Engine_Defines.h"
#include "MapTool_Enum.h"
#include "MapTool_Struct.h"
#include "MapTool_Func.h"


namespace MapTool
{
	static const unsigned int	g_iWinSizeX = { 1600 };
	static const unsigned int	g_iWinSizeY = { 900 };

	static const wchar_t*		g_strMapModelPath = { L"../../Resources/Maps/" };
	static const wchar_t*		g_strUIScenePath = { L"../../Resources/UIScene/" };
}

extern HINSTANCE				g_hInstance;
extern HWND						g_hWnd;

using namespace MapTool;