#pragma once
#include <Windows.h>
#include <process.h>

#include "GameContent_Defines.h"

#include "imgui.h"
#include "imgui_internal.h"          
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

using namespace Engine;
using namespace Client;

#include "Engine_Defines.h"
#include "AnimUITool_Enum.h"
#include "AnimUITool_Struct.h"
#include "AnimUITool_Func.h"


namespace AnimUITool
{
	static const unsigned int	g_iWinSizeX = { 1600 };
	static const unsigned int	g_iWinSizeY = { 900 };

	static const wchar_t* g_strAnimModelPath = { L"../../Resources/CHJ/AnimModel/" };
	static const wchar_t* g_strUIScenePath = { L"../../Resources/CHJ/UIScene/" };
}

extern HINSTANCE				g_hInstance;
extern HWND						g_hWnd;

using namespace std;

using namespace AnimUITool;