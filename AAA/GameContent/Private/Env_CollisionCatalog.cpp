#include "Env_CollisionCatalog.h"

#include "DataLoader.h"
#include "GameContent_Log.h"

#include <mutex>
#include <cwctype>

NS_BEGIN(Client)

namespace
{
	using RECORD_MAP = unordered_map<wstring, ENV_COLLISION_CATALOG_RECORD>;

	wstring NormalizeCatalogKey(wstring strValue)
	{
		for (wchar_t& ch : strValue)
			ch = static_cast<wchar_t>(towlower(ch));

		return strValue;
	}

	RECORD_MAP& Get_Records()
	{
		static RECORD_MAP Records;
		return Records;
	}

	wstring& Get_LoadedPath()
	{
		static wstring LoadedPath;
		return LoadedPath;
	}

	_bool& Get_Loaded()
	{
		static _bool bLoaded = false;
		return bLoaded;
	}

	mutex& Get_Mutex()
	{
		static mutex Mutex;
		return Mutex;
	}

	void Add_CandidateKey(
		const wstring& strKey,
		const ENV_COLLISION_CATALOG_RECORD& Record)
	{
		if (strKey.empty())
			return;

		const _wstring strNormalized = NormalizeCatalogKey(strKey);
		Get_Records().try_emplace(strNormalized, Record);
	}
}

HRESULT CEnv_CollisionCatalog::Load(const _wstring& strCatalogPath)
{
	lock_guard<mutex> Lock(Get_Mutex());

	if (Get_Loaded() && 0 == _wcsicmp(Get_LoadedPath().c_str(), strCatalogPath.c_str()))
		return S_OK;

	Clear();

	if (strCatalogPath.empty())
		return E_FAIL;

	string strContent;
	if (FAILED(CDataLoader::Read_Json(strCatalogPath.c_str(), &strContent)))
	{
		Log_GameContentWarning(
			"EnvCollisionCatalog load failed: path=" + WstrToStr(strCatalogPath));
		return E_FAIL;
	}

	json jRoot;
	try
	{
		jRoot = json::parse(strContent);
	}
	catch (const exception& e)
	{
		Log_GameContentWarning(
			"EnvCollisionCatalog parse failed: path=" + WstrToStr(strCatalogPath)
			+ " reason=" + e.what());
		return E_FAIL;
	}

	const json* pItems = nullptr;
	if (jRoot.contains("items") && jRoot["items"].is_array())
		pItems = &jRoot["items"];

	if (nullptr == pItems)
	{
		Log_GameContentWarning(
			"EnvCollisionCatalog has no items array: path=" + WstrToStr(strCatalogPath));
		return E_FAIL;
	}

	for (const json& jItem : *pItems)
	{
		if (!jItem.is_object())
			continue;

		if (!jItem.contains("objectName") || !jItem["objectName"].is_string())
			continue;

		ENV_COLLISION_CATALOG_RECORD Record{};
		Record.strObjectName = StrToWstr(jItem["objectName"].get<string>());

		if (jItem.contains("apxbinName") && jItem["apxbinName"].is_string())
			Record.strApxbinName = StrToWstr(jItem["apxbinName"].get<string>());

		if (jItem.contains("bfresPath") && jItem["bfresPath"].is_string())
			Record.strBfresPath = StrToWstr(jItem["bfresPath"].get<string>());

		Add_CandidateKey(Record.strObjectName, Record);

		// 일부 경로에서 ObjectName 끝의 L 제거명을 쓰는 경우에 대비한 보조 키.
		if (Record.strObjectName.size() > 1 && Record.strObjectName.back() == L'L')
		{
			const wstring strTrimmed =
				Record.strObjectName.substr(0, Record.strObjectName.size() - 1);
			Add_CandidateKey(strTrimmed, Record);
		}
	}

	Get_LoadedPath() = strCatalogPath;
	Get_Loaded() = true;

	Log_GameContentInfo(
		"EnvCollisionCatalog loaded: path=" + WstrToStr(strCatalogPath)
		+ " records=" + to_string(Get_Records().size()));

	return S_OK;
}

void CEnv_CollisionCatalog::Clear()
{
	Get_Records().clear();
	Get_LoadedPath().clear();
	Get_Loaded() = false;
}

_bool CEnv_CollisionCatalog::Is_Loaded()
{
	return Get_Loaded();
}

_bool CEnv_CollisionCatalog::Try_Find(
	const _wstring& strObjectName,
	ENV_COLLISION_CATALOG_RECORD* pOutRecord)
{
	if (!Get_Loaded() || strObjectName.empty())
		return false;

	const _wstring strKey = NormalizeCatalogKey(strObjectName);

	const auto Iter = Get_Records().find(strKey);
	if (Iter == Get_Records().end())
		return false;

	if (nullptr != pOutRecord)
		*pOutRecord = Iter->second;

	return true;
}

NS_END