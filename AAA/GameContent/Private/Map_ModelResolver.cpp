#include "Map_ModelResolver.h"

#include "GameContent_Log.h"
#include "YshModelValidator.h"

#include <cwctype>
#include <mutex>

NS_BEGIN(Client)

struct MAP_ENV_MODEL_INDEX
{
	struct ENTRY
	{
		_wstring wstrModelPath;
		_wstring wstrModelProtoTag;
	};

	unordered_map<_wstring, ENTRY> Entries;
};

namespace
{	
	using namespace std::filesystem;

	constexpr _bool kEnableYshPrevalidation = false;

	unordered_set<_wstring>& Get_RejectedModelLogSet()
	{
		static unordered_set<_wstring> s_Set;
		return s_Set;
	}

	unordered_set<_wstring>& Get_TrimmedResolveLogSet()
	{
		static unordered_set<_wstring> s_Set;
		return s_Set;
	}

	_bool Equals_NoCase(const _wstring& strLeft, const _wstring& strRight)
	{
		return 0 == _wcsicmp(strLeft.c_str(), strRight.c_str());
	}

	void Add_UniqueCandidate(const _wstring& strCandidate, vector<_wstring>* pOutCandidates)
	{
		if (nullptr == pOutCandidates || strCandidate.empty())
			return;

		auto Iter = find_if(
			pOutCandidates->begin(),
			pOutCandidates->end(),
			[&](const _wstring& strExisting)
			{
				return Equals_NoCase(strExisting, strCandidate);
			});

		if (Iter == pOutCandidates->end())
			pOutCandidates->push_back(strCandidate);
	}

	void Normalize_PathToken(_wstring* pInOut)
	{
		if (nullptr == pInOut)
			return;

		for (wchar_t& ch : *pInOut)
		{
			if (iswalnum(ch))
				continue;

			ch = L'_';
		}
	}

	_wstring To_LowerCopy(_wstring strValue)
	{
		for (wchar_t& ch : strValue)
			ch = static_cast<wchar_t>(towlower(ch));

		return strValue;
	}

	_wstring Make_EnvModelIndexKey(const filesystem::path& Root)
	{
		return To_LowerCopy(Root.lexically_normal().wstring());
	}

	unordered_map<_wstring, shared_ptr<const MAP_ENV_MODEL_INDEX>>& Get_EnvModelIndexCache()
	{
		static unordered_map<_wstring, shared_ptr<const MAP_ENV_MODEL_INDEX>> Cache;
		return Cache;
	}

	mutex& Get_EnvModelIndexMutex()
	{
		static mutex Mutex;
		return Mutex;
	}

	_bool IsValidEnvModelFile(const path& FilePath)
	{
		if (!kEnableYshPrevalidation)
			return true;

		_wstring strReason;
		if (!YshModelValidator::Validate_NonAnimYsh(FilePath, &strReason))
		{
			const _wstring strPath = FilePath.wstring();
			if (Get_RejectedModelLogSet().insert(strPath).second)
			{
				Log_GameContentWarning(
					"EnvObject ignored invalid non-anim ysh: " + WstrToStr(strPath)
					+ " reason=" + WstrToStr(strReason));
			}
			return false;
		}

		return true;
	}
}

CMap_ModelResolver::CMap_ModelResolver(const _wstring& strMapRoot)
	: m_strMapRoot{ strMapRoot }
{
}

HRESULT CMap_ModelResolver::Resolve_MapSection(
	const _wstring& strStageFolderName,
	const _wstring& strSectionName,
	_wstring* pOutModelPath,
	_wstring* pOutModelProtoTag) const
{
	if (nullptr == pOutModelPath || nullptr == pOutModelProtoTag)
		return E_FAIL;

	if (strStageFolderName.empty() || strSectionName.empty())
		return E_FAIL;

	vector<_wstring> Candidates;
	Build_MapSectionCandidates(strSectionName, &Candidates);
	if (Candidates.empty())
		return E_FAIL;

	const path SectionRoot = path(m_strMapRoot) / strStageFolderName / L"Section";

	error_code ErrorCode;
	if (!exists(SectionRoot, ErrorCode) || ErrorCode)
		return E_FAIL;

	for (const _wstring& strCandidate : Candidates)
	{
		const path CandidatePath = SectionRoot / strCandidate;

		ErrorCode.clear();
		if (exists(CandidatePath, ErrorCode) && !ErrorCode)
		{
			*pOutModelPath = CandidatePath.wstring();
			*pOutModelProtoTag = Make_MapSectionProtoTag(strStageFolderName, strSectionName);
			return S_OK;
		}
	}

	return E_FAIL;
}

HRESULT CMap_ModelResolver::Build_EnvModelCache()
{
	const path Root = path(m_strMapRoot) / L"Env";
	const _wstring strCacheKey = Make_EnvModelIndexKey(Root);

	lock_guard<mutex> Lock(Get_EnvModelIndexMutex());

	auto& Cache = Get_EnvModelIndexCache();
	const auto Iter = Cache.find(strCacheKey);
	if (Iter != Cache.end())
	{
		m_pEnvModelIndex = Iter->second;
		return S_OK;
	}

	auto pMutableIndex = make_shared<MAP_ENV_MODEL_INDEX>();

	error_code ErrorCode;
	if (!exists(Root, ErrorCode) || ErrorCode)
	{
		Log_GameContentWarning("EnvObject model root missing: " + WstrToStr(Root.wstring()));
	}
	else
	{
		for (recursive_directory_iterator Iter(Root, directory_options::skip_permission_denied, ErrorCode),
			End;
			Iter != End;
			Iter.increment(ErrorCode))
		{
			if (ErrorCode)
				break;

			if (!Iter->is_regular_file())
				continue;

			const path FilePath = Iter->path();
			if (0 != _wcsicmp(FilePath.extension().c_str(), L".ysh"))
				continue;

			if (!IsValidEnvModelFile(FilePath))
				continue;

			const _wstring strStem = To_LowerCopy(FilePath.stem().wstring());
			if (pMutableIndex->Entries.find(strStem) != pMutableIndex->Entries.end())
				continue;

			MAP_ENV_MODEL_INDEX::ENTRY Entry{};
			Entry.wstrModelPath = FilePath.wstring();
			Entry.wstrModelProtoTag = Make_EnvModelProtoTag(Root, FilePath);

			pMutableIndex->Entries.emplace(strStem, move(Entry));
		}
	}

	shared_ptr<const MAP_ENV_MODEL_INDEX> pIndex = pMutableIndex;
	Cache.emplace(strCacheKey, pIndex);
	m_pEnvModelIndex = pIndex;

	return S_OK;
}

_bool CMap_ModelResolver::Resolve_EnvObject(ENV_OBJECT_DESC* pDesc)
{
	if (nullptr == pDesc)
		return false;

	pDesc->wstrModelPath.clear();
	pDesc->wstrModelProtoTag.clear();

	if (pDesc->wstrObjectName.empty())
		return false;

	if (Resolve_EnvByKey(pDesc->wstrObjectName, pDesc))
		return true;

	if (pDesc->wstrObjectName.size() > 1 && L'L' == pDesc->wstrObjectName.back())
	{
		const _wstring strTrimmedName =
			pDesc->wstrObjectName.substr(0, pDesc->wstrObjectName.size() - 1);

		if (Resolve_EnvByKey(strTrimmedName, pDesc))
		{
			if (Get_TrimmedResolveLogSet().insert(pDesc->wstrObjectName).second)
			{
				Log_GameContentInfo(
					"EnvObject trimmed model match: object=" + WstrToStr(pDesc->wstrObjectName)
					+ " model=" + WstrToStr(strTrimmedName)
					+ " path=" + WstrToStr(pDesc->wstrModelPath));
			}

			return true;
		}
	}

	return false;
}

void CMap_ModelResolver::Clear_EnvModelCache()
{
	m_pEnvModelIndex.reset();
	Invalidate_EnvModelCache(m_strMapRoot);
}

void CMap_ModelResolver::Invalidate_EnvModelCache(const _wstring& strMapRoot)
{
	const path Root = path(strMapRoot) / L"Env";
	const _wstring strCacheKey = Make_EnvModelIndexKey(Root);

	lock_guard<mutex> Lock(Get_EnvModelIndexMutex());
	Get_EnvModelIndexCache().erase(strCacheKey);
	Get_RejectedModelLogSet().clear();
	Get_TrimmedResolveLogSet().clear();
}

void CMap_ModelResolver::Build_MapSectionCandidates(
	const _wstring& strSectionName,
	vector<_wstring>* pOutCandidates) const
{
	if (nullptr == pOutCandidates)
		return;

	pOutCandidates->clear();
	Add_UniqueCandidate(strSectionName + L".ysh", pOutCandidates);

	if (0 == strSectionName.rfind(L"Land_", 0))
		Add_UniqueCandidate(strSectionName.substr(5) + L".ysh", pOutCandidates);
}

_wstring CMap_ModelResolver::Make_MapSectionProtoTag(
	const _wstring& strStageFolderName,
	const _wstring& strSectionName) const
{
	return L"Prototype_Component_Model_MapSection_" + strStageFolderName + L"_" + strSectionName;
}

_wstring CMap_ModelResolver::Make_EnvModelProtoTag(
	const filesystem::path& Root,
	const filesystem::path& FilePath) const
{
	error_code ErrorCode;
	path RelativePath = relative(FilePath, Root, ErrorCode);
	if (ErrorCode)
		RelativePath = FilePath.filename();

	RelativePath.replace_extension();

	_wstring strToken = RelativePath.wstring();
	Normalize_PathToken(&strToken);

	return L"Prototype_Component_Model_Env_" + strToken;
}

_bool CMap_ModelResolver::Resolve_EnvByKey(const _wstring& strKey, ENV_OBJECT_DESC* pDesc) const
{
	if (nullptr == pDesc || strKey.empty())
		return false;

	const shared_ptr<const MAP_ENV_MODEL_INDEX> pIndex = m_pEnvModelIndex;
	if (nullptr == pIndex)
		return false;

	const auto Iter = pIndex->Entries.find(To_LowerCopy(strKey));
	if (Iter == pIndex->Entries.end())
		return false;

	pDesc->wstrModelPath = Iter->second.wstrModelPath;
	pDesc->wstrModelProtoTag = Iter->second.wstrModelProtoTag;
	return true;
}

CMap_ModelResolver* CMap_ModelResolver::Create(const _wstring& strMapRoot)
{
	return new CMap_ModelResolver(strMapRoot);
}

void CMap_ModelResolver::Free()
{
	__super::Free();
}

NS_END