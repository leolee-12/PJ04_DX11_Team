#include "DataExporter.h"
#include "Level.h"
#include "GameObject.h"
#include <fstream>


json CDataExporter::Float4_ToJson(const _float4& v)
{
	return json::array({ v.x, v.y, v.z, v.w });
}

HRESULT CDataExporter::Write_JsonFile(const _tchar* pFilePath, const json& jData)
{
	ofstream file(pFilePath);
	if (!file.is_open()) return E_FAIL;

	file << jData.dump(4);
	file.close();
	return S_OK;
}
