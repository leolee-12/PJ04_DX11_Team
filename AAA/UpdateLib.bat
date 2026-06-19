@echo off
:: 명령어		옵션			원본 파일의 위치						사본 파일을 저장할 위치

xcopy			/y /D /s	.\Engine\Public\*.*							.\EngineSDK\Inc\
xcopy			/y /D		.\Engine\Bin\Engine.lib						.\EngineSDK\lib\

xcopy			/y /D /s	.\GameContent\Public\nlohmann\json.hpp		.\ContentSDK\Inc\nlohmann\
xcopy			/y /D		.\GameContent\Bin\GameContent.lib			.\ContentSDK\lib\
xcopy			/y /D		.\GameContent\Public\*.h					.\ContentSDK\Inc\

xcopy			/y /D		.\Engine\ShaderFiles\*.*					.\GameContent\ShaderFiles\

xcopy			/y /D		.\Engine\Bin\*.dll							.\Launcher\Bin\
xcopy			/y /D		.\Engine\ThirdPartyLib\*.dll				.\Launcher\Bin\
xcopy			/y /D		.\GameContent\Bin\*.dll						.\Launcher\Bin\
xcopy			/y /D /s	.\GameContent\ShaderFiles\*.*				.\Launcher\Bin\ShaderFiles\

xcopy			/y /D		.\Engine\Bin\*.dll							.\Editor\Bin\
xcopy			/y /D		.\Engine\ThirdPartyLib\*.dll				.\Editor\Bin\
xcopy			/y /D		.\GameContent\Bin\*.dll						.\Editor\Bin\
xcopy			/y /D /s	.\GameContent\ShaderFiles\*.*				.\Editor\Bin\ShaderFiles\


xcopy			/y /D		.\Engine\Bin\*.dll							.\AnimUITool\Bin\
xcopy			/y /D		.\Engine\ThirdPartyLib\*.dll				.\AnimUITool\Bin\
xcopy			/y /D		.\GameContent\Bin\*.dll						.\AnimUITool\Bin\
xcopy			/y /D /s    .\GameContent\ShaderFiles\*.*				.\AnimUITool\Bin\ShaderFiles\


xcopy			/y /D		.\Engine\Bin\*.dll							.\MapTool\Bin\
xcopy			/y /D		.\Engine\ThirdPartyLib\*.dll				.\MapTool\Bin\
xcopy			/y /D		.\GameContent\Bin\*.dll						.\MapTool\Bin\
xcopy			/y /D /s	.\GameContent\ShaderFiles\*.*				.\MapTool\Bin\ShaderFiles\


xcopy			/y /D		.\Engine\Bin\*.dll							.\CameraTool\Bin\
xcopy			/y /D		.\Engine\ThirdPartyLib\*.dll				.\CameraTool\Bin\
xcopy			/y /D		.\GameContent\Bin\*.dll						.\CameraTool\Bin\
xcopy			/y /D /s	.\GameContent\ShaderFiles\*.*				.\CameraTool\Bin\ShaderFiles\

exit /b 0