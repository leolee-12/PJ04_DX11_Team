#pragma once
#include "Base.h"
#include "Map_Override.h"

NS_BEGIN(Client)

class CMap_DeltaIO
{
	static HRESULT Resolve_DeltaPathFromManifest(const _wstring& strManifestPath, _wstring* pOutDeltaPath);

	static HRESULT Load_DeltaFromManifest(const _wstring& strManifestPath, MAP_OVERRIDE_DESC* pOutDesc, _wstring* pOutDeltaPath = nullptr);

	static HRESULT Save_DeltaToPath(const _wstring& strDeltaPath, const MAP_OVERRIDE_DESC& Desc);
};

NS_END