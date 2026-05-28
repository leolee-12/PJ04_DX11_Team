#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)

class ENGINE_DLL CDataLoader final
{
private:
	CDataLoader() = default;
	~CDataLoader() = default;

public:
	static HRESULT Read_Json(const _tchar* strFilePath, string* pOutContnet);
	static void Parse_Float4(const json& jArray, _float4* pOutFloat4);
	static HRESULT Read_ysh(const _tchar* strFilePath, MODEL_DATA& out);

private:
	static void Read_BoneHierarchy(ifstream& file, vector<BONE_DATA>& bones, _int parentIdx);
};

NS_END

