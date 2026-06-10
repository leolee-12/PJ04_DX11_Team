#pragma once
#include <Windows.h>
#include <process.h>

#include "GameContent_Defines.h"


namespace Editor
{
	static const unsigned int		g_iWinSizeX = { 1600 };
	static const unsigned int		g_iWinSizeY = { 900 };

	static const wchar_t*			g_strEditPath = {L"../../Resources/LevelData/"};
	static const wchar_t*			g_strLiveobjectPath = { L"../../Resources/LevelData/LiveObject/" };

	enum class EDIT_LEVEL
	{
		STATIC,
		EDIT,
		END
	};
}

extern HINSTANCE	g_hInstance;
extern HWND			g_hWndEditor;

using namespace Editor;