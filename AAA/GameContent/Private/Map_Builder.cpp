#include "Map_Builder.h"
#include "GameContent_Log.h"
#include "Map_ModelResolver.h"
#include "MapGimmick_Defines.h"
#include "Map_Parser.h"
#include "Map_EditFile.h"
#include "Env_CollisionCatalog.h"

#include "DataLoader.h"

#define ENABLE_ENV_MISSING_MODEL_LOG

NS_BEGIN(Client)

namespace
{
#ifdef ENABLE_ENV_MISSING_MODEL_LOG
	const char* ToString(ENV_SOURCE_TYPE eSourceType)
	{
		switch (eSourceType)
		{
		case ENV_SOURCE_TYPE::DECOR_DECOR: return "DECOR_DECOR";
		case ENV_SOURCE_TYPE::TOY_OBJ: return "TOY_OBJ";
		case ENV_SOURCE_TYPE::DECOR_OBJ: return "DECOR_OBJ";
		case ENV_SOURCE_TYPE::UNKNOWN: return "UNKNOWN";
		default: return "END";
		}
	}

	const char* ToString(ENV_OBJECT_KIND eKind)
	{
		switch (eKind)
		{
		case ENV_OBJECT_KIND::STATIC: return "STATIC";
		case ENV_OBJECT_KIND::INTERACT: return "INTERACT";
		case ENV_OBJECT_KIND::EFFECT: return "EFFECT";
		case ENV_OBJECT_KIND::UNKNOWN: return "UNKNOWN";
		default: return "END";
		}
	}

	void Log_MissingEnvModelDetail(const ENV_OBJECT_DESC& Desc)
	{
		Log_GameContentWarning(
			"Map builder skipped env without model: source="
			+ WstrToStr(Desc.wstrSourceFile)
			+ " section=" + WstrToStr(Desc.wstrSection)
			+ " entry=" + WstrToStr(Desc.wstrEntryKey)
			+ " uid=" + to_string(Desc.iUid)
			+ " object=" + WstrToStr(Desc.wstrObjectName)
			+ " sourceType=" + ToString(Desc.eSourceType)
			+ " kind=" + ToString(Desc.eKind));
	}
#endif
}

CMap_Builder::CMap_Builder(CMap_ModelResolver* pResolver)
	: m_pResolver{ pResolver }
{
	Safe_AddRef(m_pResolver);
}

HRESULT CMap_Builder::Build_FromManifest(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage)
{
	return Build_FromManifest(strManifestPath, MAP_PACKAGE_BUILD_OPTIONS{}, pOutPackage);
}

HRESULT CMap_Builder::Build_FromManifest(const _wstring& strManifestPath, const MAP_PACKAGE_BUILD_OPTIONS& Options, MAP_PACKAGE* pOutPackage)
{
	if (nullptr == pOutPackage || nullptr == m_pResolver)
		return E_FAIL;

	MAP_MANIFEST_DESC Manifest{};
	if (FAILED(CMap_Parser::Parse_Manifest(strManifestPath, &Manifest)))
		return E_FAIL;

	if (Options.bBuildEnv && !Manifest.strDecorCollisionCatalogPath.empty())
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

	if (Options.bBuildLevelDesignPaths)
		pOutPackage->LevelDesignJsonPaths = Manifest.LevelDesignJsonPaths;

	if (Options.bBuildStage)
	{
		if (FAILED(Build_StageDesc(Manifest, &pOutPackage->StageDesc)))
			return E_FAIL;
	}

	if (Options.bBuildEnv)
	{
		if (FAILED(Build_EnvDescs(
			Manifest,
			&pOutPackage->EnvObjectDescs,
			&pOutPackage->EnvJsonPaths)))
		{
			return E_FAIL;
		}

		if (FAILED(Validate_And_Filter(pOutPackage)))
			return E_FAIL;
	}

	if (Options.bApplyDelta && !Manifest.strDeltaPath.empty())
	{
		string strDeltaContent;
		if (FAILED(CDataLoader::Read_Json(Manifest.strDeltaPath.c_str(), &strDeltaContent)))
			return E_FAIL;

		MAP_EDIT_CHANGE OverrideDesc{};
		json jDelta = json::parse(strDeltaContent);
		if (FAILED(CMap_EditFile::Load_Change(jDelta, &OverrideDesc)))
			return E_FAIL;

		MAP_EDIT_APPLY_OPTIONS ApplyOptions{};
		ApplyOptions.bApplyStage = Options.bBuildStage;
		ApplyOptions.bApplyEnv = Options.bBuildEnv;
		ApplyOptions.bApplyAddedObjects = Options.bBuildEnv || Options.bBuildLevelDesignPaths;

		if (FAILED(CMap_EditFile::Apply_Change(pOutPackage, OverrideDesc, ApplyOptions)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMap_Builder::Build_StageDesc(const MAP_MANIFEST_DESC& Manifest, MAP_STAGE_DESC* pOutStageDesc)
{
	if (nullptr == pOutStageDesc || nullptr == m_pResolver)
		return E_FAIL;

	const size_t iSectionCount = Manifest.Sections.size();

	pOutStageDesc->strStageName = Manifest.strStageName;
	pOutStageDesc->iSectionProtoLevel = 0;
	pOutStageDesc->SectionDescs.clear();
	pOutStageDesc->SectionDescs.reserve(iSectionCount);

	for (const MAP_MANIFEST_SECTION& Section : Manifest.Sections)
	{
		const _wstring& strSectionName = Section.strName;

		if (Is_MapGimmickSection(Manifest.strStageName, strSectionName))
			continue;

		_wstring wstrModelPath;
		_wstring wstrModelProtoTag;

		if (FAILED(m_pResolver->Resolve_MapSection(
			Manifest.strStageFolderName,
			strSectionName,
			&wstrModelPath,
			&wstrModelProtoTag)))
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
		Desc.wstrModelPath = wstrModelPath;
		Desc.wstrModelProtoTag = wstrModelProtoTag;
		Desc.iModelProtoLevel = 0;
		Desc.bRenderable = true;
		Desc.bUseCollMesh = true;

		pOutStageDesc->SectionDescs.push_back(Desc);
	}

	return S_OK;
}

HRESULT CMap_Builder::Build_EnvDescs(const MAP_MANIFEST_DESC& Manifest, vector<ENV_OBJECT_DESC>* pOutEnvDescs, vector<_wstring>* pOutJsonPaths)
{
	if (nullptr == pOutEnvDescs || nullptr == pOutJsonPaths || nullptr == m_pResolver)
		return E_FAIL;

	pOutEnvDescs->clear();
	pOutJsonPaths->clear();

	if (FAILED(m_pResolver->Build_EnvModelCache()))
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
			if (EnvObject_NeedsModel(Desc))
				m_pResolver->Resolve_EnvObject(&Desc);

			Desc.tCollision.bCookCollMesh = Desc.tCollision.bHasCollMesh;
			Desc.tCollision.bUseCollMesh = false;

			Desc.tRender.bUseShadow = Desc.tRender.bHasShadow;

			if (Desc.tRender.bIsDecal)
			{
				Desc.tRender.bUseShadow = false;

				Desc.tCollision.eColliderKind = ENV_COLLIDER_KIND::NONE;
				Desc.tCollision.bHasCollMesh = false;
				Desc.tCollision.bCookCollMesh = false;
				Desc.tCollision.bUseCollMesh = false;
			}

			pOutEnvDescs->push_back(std::move(Desc));
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

	_uint iSkippedMissingModel = 0;

	for (ENV_OBJECT_DESC& Desc : pPackage->EnvObjectDescs)
	{
		if (EnvObject_NeedsModel(Desc)
			&& (Desc.wstrModelPath.empty() || Desc.wstrModelProtoTag.empty()))
		{
			++iSkippedMissingModel;

#ifdef ENABLE_ENV_MISSING_MODEL_LOG
			Log_MissingEnvModelDetail(Desc);
#endif
			continue;
		}

		Filtered.push_back(std::move(Desc));
	}

	pPackage->EnvObjectDescs.swap(Filtered);
	pPackage->iEnvSkippedMissingModel = iSkippedMissingModel;

#ifdef ENABLE_ENV_MISSING_MODEL_LOG
	if (0 != iSkippedMissingModel)
	{
		Log_GameContentWarning(
			"Map builder skipped env without model: count="
			+ to_string(iSkippedMissingModel));
	}
#endif

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