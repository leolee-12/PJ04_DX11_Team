@echo off
:: usage: SortFilters.bat [options...]
::   (no option)   sort every *.vcxproj.filters under AAA, keeping a <file>.bak backup
::   --check       report files that need sorting, without modifying them (no backup made)
:: exit code: 0 = already sorted / sorted now, 1 = --check found unsorted files, 2 = error
:: NOTE: close Visual Studio (or unload projects) before sorting in place.

setlocal
:: prefer the Windows py launcher, fall back to python on PATH
set "PY_CMD=py -3"
where py >nul 2>nul
if errorlevel 1 set "PY_CMD=python"

:: --backup is always applied; it is a no-op in --check mode
:: trailing "." keeps the closing quote from being escaped by the trailing backslash
%PY_CMD% "%~dp0Tools\sort_vcxproj_filters.py" "%~dp0." --backup %*
set "RESULT=%errorlevel%"

:: keep the window open when launched by double-click (no arguments)
if "%~1"=="" pause

exit /b %RESULT%
