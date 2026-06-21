#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Client)

class CMap_ModelResolver final : public CBase
{
private:
	CMap_ModelResolver(const _wstring& strMapRoot);
	virtual ~CMap_ModelResolver() = default;

public:
	HRESULT Resolve_MapSection(const _wstring& strStageFolderName, const _wstring& strSectionName, _wstring* pOutModelPath, _wstring* pOutModelProtoTag) const;
	HRESULT Build_EnvModelCache();
	_bool Resolve_EnvObject(ENV_OBJECT_DESC* pDesc);
	void Clear_EnvModelCache();

private:
	void Build_MapSectionCandidates(const _wstring& strSectionName, vector<_wstring>* pOutCandidates) const;
	_wstring Make_MapSectionProtoTag(const _wstring& strStageFolderName, const _wstring& strSectionName) const;
	_wstring Make_EnvModelProtoTag(const filesystem::path& Root, const filesystem::path& FilePath) const;
	_bool Resolve_EnvByKey(const _wstring& strKey, ENV_OBJECT_DESC* pDesc) const;

private:
	struct ENV_MODEL_ENTRY
	{
		_wstring wstrModelPath;
		_wstring wstrModelProtoTag;
	};

private:
	_wstring m_strMapRoot;
	filesystem::path m_EnvCacheRoot;
	unordered_map<_wstring, ENV_MODEL_ENTRY> m_EnvModelCache;

public:
	static CMap_ModelResolver* Create(const _wstring& strMapRoot = L"../../Resources/Map");

private:
	virtual void Free() override;
};

NS_END