@echo off
:: usage: UpdateLib.bat <Configuration>   (Debug | Release)
set CFG=%~1
if "%CFG%"=="" set CFG=Debug

:: command   options       source                                  destination
xcopy /y /D /s  .\Engine\Public\*.*                          .\EngineSDK\Inc\
xcopy /y /D     .\Engine\Bin\%CFG%\Engine.lib                .\EngineSDK\Lib\%CFG%\

xcopy /y /D     .\GameContent\Public\*.h                     .\ContentSDK\Inc\
xcopy /y /D /s  .\GameContent\Public\nlohmann\json.hpp       .\ContentSDK\Inc\nlohmann\
xcopy /y /D     .\GameContent\Bin\%CFG%\GameContent.lib      .\ContentSDK\Lib\%CFG%\

xcopy /y /D     .\Engine\ShaderFiles\*.*                     .\GameContent\ShaderFiles\

xcopy /y /D     .\Engine\Bin\%CFG%\*.dll                     .\Launcher\Bin\%CFG%\
xcopy /y /D     .\GameContent\Bin\%CFG%\*.dll                .\Launcher\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\PhysX\%CFG%\*.dll     .\Launcher\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\%CFG%\*.dll           .\Launcher\Bin\%CFG%\
xcopy /y /D /s  .\GameContent\ShaderFiles\*.*                .\Launcher\Bin\ShaderFiles\

xcopy /y /D     .\Engine\Bin\%CFG%\*.dll                     .\Editor\Bin\%CFG%\
xcopy /y /D     .\GameContent\Bin\%CFG%\*.dll                .\Editor\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\PhysX\%CFG%\*.dll     .\Editor\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\%CFG%\*.dll           .\Editor\Bin\%CFG%\
xcopy /y /D /s  .\GameContent\ShaderFiles\*.*                .\Editor\Bin\ShaderFiles\

xcopy /y /D     .\Engine\Bin\%CFG%\*.dll                     .\AnimUITool\Bin\%CFG%\
xcopy /y /D     .\GameContent\Bin\%CFG%\*.dll                .\AnimUITool\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\PhysX\%CFG%\*.dll     .\AnimUITool\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\%CFG%\*.dll           .\AnimUITool\Bin\%CFG%\
xcopy /y /D /s  .\GameContent\ShaderFiles\*.*                .\AnimUITool\Bin\ShaderFiles\

xcopy /y /D     .\Engine\Bin\%CFG%\*.dll                     .\MapTool\Bin\%CFG%\
xcopy /y /D     .\GameContent\Bin\%CFG%\*.dll                .\MapTool\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\PhysX\%CFG%\*.dll     .\MapTool\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\%CFG%\*.dll           .\MapTool\Bin\%CFG%\
xcopy /y /D /s  .\GameContent\ShaderFiles\*.*                .\MapTool\Bin\ShaderFiles\

xcopy /y /D     .\Engine\Bin\%CFG%\*.dll                     .\CameraTool\Bin\%CFG%\
xcopy /y /D     .\GameContent\Bin\%CFG%\*.dll                .\CameraTool\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\PhysX\%CFG%\*.dll     .\CameraTool\Bin\%CFG%\
xcopy /y /D     .\Engine\ThirdPartyLib\%CFG%\*.dll			 .\CameraTool\Bin\%CFG%\
xcopy /y /D /s  .\GameContent\ShaderFiles\*.*                .\CameraTool\Bin\ShaderFiles\

exit /b 0