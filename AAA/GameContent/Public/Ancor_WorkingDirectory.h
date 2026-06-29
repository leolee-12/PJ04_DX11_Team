#pragma once
#include <Windows.h>

void Anchor_WorkingDirectory()
{
    wchar_t szExe[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, szExe, MAX_PATH);
    // szExe = ...\Launcher\Bin\Release\xxx.exe

    wchar_t* pSlash = wcsrchr(szExe, L'\\');
    if (pSlash)
        *pSlash = L'\0';                       // ...\Launcher\Bin\Release

    SetCurrentDirectoryW(szExe);
    SetCurrentDirectoryW(L"..\\..\\Default");  // ...\Launcher\Default ∑Œ ∞Ì¡§
}