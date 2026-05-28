// 명령어			옵션		원본 파일의 위치			사본 파일을 저장할 위치

xcopy			/y /s	.\Engine\Public\*.*					.\EngineSDK\Inc\
xcopy			/y		.\Engine\Bin\Engine.lib				.\EngineSDK\lib\

xcopy			/y /s	.\GameContent\Public\nlohmann\json.hpp	.\ContentSDK\Inc\nlohmann\
xcopy			/y		.\GameContent\Bin\GameContent.lib		.\ContentSDK\lib\
xcopy			/y		.\GameContent\Public\*.h				.\ContentSDK\Inc\

xcopy			/y		.\Engine\Bin\*.dll						.\Launcher\Bin\
xcopy			/y		.\Engine\Bin\*.dll						.\Editor\Bin\

xcopy			/y		.\Engine\ThirdPartyLib\*.dll			.\Launcher\Bin\
xcopy			/y		.\Engine\ThirdPartyLib\*.dll			.\Editor\Bin\

xcopy			/y		.\Engine\Bin\*.dll						.\FBX_Converter\Bin\
xcopy			/y		.\GameContent\Bin\*.dll					.\Launcher\Bin\
xcopy			/y		.\GameContent\Bin\*.dll					.\Editor\Bin\
xcopy			/y		.\Engine\ShaderFiles\*.*				.\GameContent\ShaderFiles\
xcopy			/y /s	.\GameContent\ShaderFiles\*.*			.\Launcher\Bin\ShaderFiles\
xcopy			/y /s	.\GameContent\ShaderFiles\*.*			.\Editor\Bin\ShaderFiles\

