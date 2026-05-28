#pragma once

#include "GameContent_Defines.h"
#include <Windows.h>
#include <process.h>

namespace Launcher
{
	static const unsigned int		g_iWinSizeX = { 1600 };
	static const unsigned int		g_iWinSizeY = { 900 };
}

extern HINSTANCE	g_hInstance;
extern HWND			g_hWnd;

using namespace Launcher;