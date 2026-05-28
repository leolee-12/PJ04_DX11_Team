#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CLevel;
class CGameInstance_Proxy;

class ENGINE_DLL CDataExporter final
{
private:
	CDataExporter() = default;
	~CDataExporter() = default;

public:
	static json Float4_ToJson(const _float4& v);
	static HRESULT Write_JsonFile(const _tchar* pFilePath, const json& jData);
	
};

NS_END

