#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

#include <memory>

NS_BEGIN(Client)
struct MAP_ENV_MODEL_INDEX;

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

	static void Invalidate_EnvModelCache(const _wstring& strMapRoot = L"../../Resources/Map");

private:
	void Build_MapSectionCandidates(const _wstring& strSectionName, vector<_wstring>* pOutCandidates) const;
	_wstring Make_MapSectionProtoTag(const _wstring& strStageFolderName, const _wstring& strSectionName) const;
	_wstring Make_EnvModelProtoTag(const filesystem::path& Root, const filesystem::path& FilePath) const;
	_bool Resolve_EnvByKey(const _wstring& strKey, ENV_OBJECT_DESC* pDesc) const;

private:
	_wstring m_strMapRoot;
	shared_ptr<const MAP_ENV_MODEL_INDEX> m_pEnvModelIndex;

public:
	static CMap_ModelResolver* Create(const _wstring& strMapRoot = L"../../Resources/Map");

private:
	virtual void Free() override;
};

NS_END