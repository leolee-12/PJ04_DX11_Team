#pragma once
#include <Windows.h>
#include <process.h>

#include "GameContent_Defines.h"

using namespace Engine;

#include "Engine_Defines.h"
#include "AnimUITool_Enum.h"
#include "AnimUITool_Struct.h"
#include "AnimUITool_Func.h"


namespace AnimUITool
{
	static const unsigned int	g_iWinSizeX = { 1600 };
	static const unsigned int	g_iWinSizeY = { 900 };

	static const wchar_t*		g_strAnimModelPath = { L"../../Resources/AnimModel/" };
	static const wchar_t*		g_strUIScenePath = { L"../../Resources/UIScene/" };
}

extern HINSTANCE				g_hInstance;
extern HWND						g_hWnd;

using namespace AnimUITool;