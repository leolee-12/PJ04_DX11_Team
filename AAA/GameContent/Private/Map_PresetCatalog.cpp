#include "Map_PresetCatalog.h"

#include <filesystem>

NS_BEGIN(Client)

namespace
{
	using namespace std::filesystem;

	constexpr _tchar kMapModelRoot[] = L"../../Resources/Map";

	struct MAP_PRESET_CATALOG_ENTRY
	{
		const _char* pLabel;
		const _tchar* pStageFolderName;
	};

	constexpr MAP_PRESET_CATALOG_ENTRY kPresetCatalog[] =
	{
		{ "Stage1-1", L"Stage1-1", },
		{ "Stage1-2", L"Stage1-2", },

		{ "Stage3-1", L"Stage3-1", },
		{ "Stage3-2", L"Stage3-2", },
		{ "Stage3-3", L"Stage3-3", },

		{ "BossMap1", L"BossMap1", },

		{ "Town", L"Town", },
		{ "WMap", L"WMap", },
		{ "Town2", L"Town2", },
	};

	const MAP_PRESET_CATALOG_ENTRY* Get_PresetEntry(_uint iPresetIndex)
	{
		if (iPresetIndex >= _countof(kPresetCatalog))
			return nullptr;

		return &kPresetCatalog[iPresetIndex];
	}
}

_uint CMap_PresetCatalog::Get_Count()
{
	return static_cast<_uint>(_countof(kPresetCatalog));
}

const _char* CMap_PresetCatalog::Get_Label(_uint iPresetIndex)
{
	const MAP_PRESET_CATALOG_ENTRY* pEntry = Get_PresetEntry(iPresetIndex);
	return nullptr != pEntry ? pEntry->pLabel : "Unknown";
}

HRESULT CMap_PresetCatalog::Get_ManifestPath(_uint iPresetIndex, _wstring* pOutManifestPath)
{
	if (nullptr == pOutManifestPath)
		return E_FAIL;

	pOutManifestPath->clear();

	const MAP_PRESET_CATALOG_ENTRY* pEntry = Get_PresetEntry(iPresetIndex);
	if (nullptr == pEntry)
		return E_FAIL;

	*pOutManifestPath = (path(kMapModelRoot) / pEntry->pStageFolderName /
		(_wstring(pEntry->pStageFolderName) + L"_Manifest.json")).lexically_normal().wstring();
	return S_OK;
}

NS_END