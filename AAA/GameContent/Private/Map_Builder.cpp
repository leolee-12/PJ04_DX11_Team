#include "Map_Builder.h"
#include "GameContent_Log.h"
#include "Map_ModelResolver.h"
#include "Map_Parser.h"
#include "Map_Override.h"
#include "Env_CollisionCatalog.h"

#include "DataLoader.h"

NS_BEGIN(Client)

namespace
{
	_bool Needs_Model(const ENV_OBJECT_DESC& Desc)
	{
		return Desc.eKind == ENV_OBJECT_KIND::STATIC
			|| Desc.eKind == ENV_OBJECT_KIND::INTERACT;
	}

	_wstring Describe_EnvObject(const ENV_OBJECT_DESC& Desc)
	{
		if (!Desc.strObjectName.empty())
			return Desc.strObjectName;

		if (!Desc.strEntryKey.empty())
			return Desc.strEntryKey;

		return L"<unknown>";
	}
}

CMap_Builder::CMap_Builder(CMap_ModelResolver* pResolver)
	: m_pResolver{ pResolver }
{
	Safe_AddRef(m_pResolver);
}

HRESULT CMap_Builder::Build_FromManifest(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage)
{
	if (nullptr == pOutPackage || nullptr == m_pResolver)
		return E_FAIL;

	MAP_MANIFEST_DESC Manifest{};
	if (FAILED(CMap_Parser::Parse_Manifest(strManifestPath, &Manifest)))
		return E_FAIL;

	if (!Manifest.strDecorCollisionCatalogPath.empty())
	{
		if (FAILED(CEnv_CollisionCatalog::Load(Manifest.strDecorCollisionCatalogPath)))
		{
			// 카탈로그가 없으면 맵 로드를 막지는 않되,
			// Decor 모델 메쉬 충돌은 모두 꺼지는 쪽으로 안전하게 간다.
			CEnv_CollisionCatalog::Clear();

			Log_GameContentWarning(
				"Decor collision catalog unavailable. Decor model mesh collision will be disabled. path="
				+ WstrToStr(Manifest.strDecorCollisionCatalogPath));
		}
	}

	*pOutPackage = {};

	if (FAILED(Build_StageDesc(Manifest, &pOutPackage->StageDesc)))
		return E_FAIL;

	if (FAILED(Build_EnvDescs(
		Manifest,
		&pOutPackage->EnvObjectDescs,
		&pOutPackage->EnvJsonPaths)))
	{
		return E_FAIL;
	}

	if (FAILED(Validate_And_Filter(pOutPackage)))
		return E_FAIL;

	if (!Manifest.strDeltaPath.empty())
	{
		string strDeltaContent;
		if (FAILED(CDataLoader::Read_Json(Manifest.strDeltaPath.c_str(), &strDeltaContent)))
			return E_FAIL;

		MAP_EDIT_CHANGE OverrideDesc{};
		json jDelta = json::parse(strDeltaContent);
		if (FAILED(CMap_Override::Deserialize(jDelta, &OverrideDesc)))
			return E_FAIL;

		if (FAILED(CMap_Override::Apply(pOutPackage, OverrideDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMap_Builder::Build_StageDesc(const MAP_MANIFEST_DESC& Manifest, MAP_STAGE_DESC* pOutStageDesc)
{
	if (nullptr == pOutStageDesc || nullptr == m_pResolver)
		return E_FAIL;

	const size_t iSectionCount = Manifest.SectionNames.size();
	if (Manifest.SectionTypes.size() != iSectionCount
		|| Manifest.SectionRenderIDs.size() != iSectionCount)
	{
		return E_FAIL;
	}

	pOutStageDesc->strStageName = Manifest.strStageName;
	pOutStageDesc->iSectionProtoLevel = 0;
	pOutStageDesc->SectionDescs.clear();
	pOutStageDesc->SectionDescs.reserve(iSectionCount);

	for (size_t i = 0; i < iSectionCount; ++i)
	{
		const _wstring& strSectionName = Manifest.SectionNames[i];

		_wstring strModelPath;
		_wstring strModelProtoTag;

		if (FAILED(m_pResolver->Resolve_MapSection(
			Manifest.strStageFolderName,
			strSectionName,
			&strModelPath,
			&strModelProtoTag)))
		{
			Log_GameContentWarning(
				"Map builder section model missing: stage="
				+ WstrToStr(Manifest.strStageFolderName)
				+ " section="
				+ WstrToStr(strSectionName));
			return E_FAIL;
		}

		MAP_SECTION_DESC Desc{};
		Desc.strSectionName = strSectionName;
		Desc.strModelPath = strModelPath;
		Desc.strModelProtoTag = strModelProtoTag;
		Desc.iModelProtoLevel = 0;
		Desc.eSectionType = Manifest.SectionTypes[i];
		Desc.eRenderID = Manifest.SectionRenderIDs[i];
		Desc.bRenderable = true;
		Desc.bCastShadow = true;
		Desc.bEnableCulling = true;

		pOutStageDesc->SectionDescs.push_back(Desc);
	}

	return S_OK;
}

HRESULT CMap_Builder::Build_EnvDescs(
	const MAP_MANIFEST_DESC& Manifest,
	vector<ENV_OBJECT_DESC>* pOutEnvDescs,
	vector<_wstring>* pOutJsonPaths)
{
	if (nullptr == pOutEnvDescs || nullptr == pOutJsonPaths || nullptr == m_pResolver)
		return E_FAIL;

	pOutEnvDescs->clear();
	pOutJsonPaths->clear();

	if (FAILED(m_pResolver->Build_EnvModelCache(Manifest.strStageFolderName)))
		return E_FAIL;

	for (const _wstring& strJsonPath : Manifest.EnvJsonPaths)
	{
		vector<ENV_OBJECT_DESC> JsonDescs;
		if (FAILED(CMap_Parser::Parse_EnvJson(strJsonPath, &JsonDescs)))
		{
			Log_GameContentWarning(
				"Map builder env json parse failed: " + WstrToStr(strJsonPath));
			return E_FAIL;
		}

		for (ENV_OBJECT_DESC& Desc : JsonDescs)
		{
			m_pResolver->Resolve_EnvObject(&Desc);
			pOutEnvDescs->push_back(Desc);
		}

		pOutJsonPaths->push_back(strJsonPath);
	}

	return S_OK;
}

HRESULT CMap_Builder::Validate_And_Filter(MAP_PACKAGE* pPackage)
{
	if (nullptr == pPackage)
		return E_FAIL;

	vector<ENV_OBJECT_DESC> Filtered;
	Filtered.reserve(pPackage->EnvObjectDescs.size());

	for (const ENV_OBJECT_DESC& Desc : pPackage->EnvObjectDescs)
	{
		if (Needs_Model(Desc)
			&& (Desc.strModelPath.empty() || Desc.strModelProtoTag.empty()))
		{
			Log_GameContentWarning(
				"Map builder skipped env without model: object="
				+ WstrToStr(Describe_EnvObject(Desc)));
			continue;
		}

		Filtered.push_back(Desc);
	}

	pPackage->EnvObjectDescs.swap(Filtered);
	return S_OK;
}

CMap_Builder* CMap_Builder::Create(CMap_ModelResolver* pResolver)
{
	return new CMap_Builder(pResolver);
}

void CMap_Builder::Free()
{
	Safe_Release(m_pResolver);

	__super::Free();
}

NS_END