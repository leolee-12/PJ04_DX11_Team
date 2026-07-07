#include "Map_Parser.h"
#include "GameContent_Log.h"
#include "Env_CollisionCatalog.h"

#include "DataLoader.h"

#include <cctype>
#include <exception>

NS_BEGIN(Client)

namespace
{
	vector<string> Split_DottedPath(const string& strPath)
	{
		vector<string> Parts;

		size_t iStart = 0;
		while (iStart < strPath.size())
		{
			const size_t iDot = strPath.find('.', iStart);
			if (string::npos == iDot)
			{
				Parts.push_back(strPath.substr(iStart));
				break;
			}

			Parts.push_back(strPath.substr(iStart, iDot - iStart));
			iStart = iDot + 1;
		}

		return Parts;
	}

	_string To_UpperCopy(_string strValue)
	{
		for (char& ch : strValue)
			ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));

		return strValue;
	}

	_bool Is_SameText(const wstring& A, const wchar_t* B)
	{
		return 0 == _wcsicmp(A.c_str(), B);
	}

	_bool Is_SkippedEnvSourceFile(const wstring& wstrSourceFile)
	{
		return Is_SameText(wstrSourceFile, L"Toy_Decor.bin");
	}

	void Resolve_EnvColliderKind(ENV_OBJECT_DESC* pDesc)
	{
		if (nullptr == pDesc)
			return;

		ENV_COLLISION_DESC& Collision = pDesc->tCollision;

		Collision.eColliderKind = ENV_COLLIDER_KIND::NONE;

		Collision.bHasCollMesh = false;
		Collision.bCookCollMesh = false;
		Collision.bUseCollMesh = false;

		Collision.bCatalogCollisionChecked = false;
		Collision.bHasDecorCollisionApxbin = false;
		Collision.strDecorCollisionApxbinName.clear();
		Collision.strDecorCollisionBfresPath.clear();

		if (pDesc->eKind == ENV_OBJECT_KIND::EFFECT)
			return;

		// Decor 계열 모델 메쉬 충돌은 원본 IsInvalidCollision 플래그 대신
		// DecorCollisionCatalog의 objectName 존재 여부로 판단한다.
		if (pDesc->eSourceType == ENV_SOURCE_TYPE::DECOR_DECOR)
		{
			Collision.bCatalogCollisionChecked = true;

			ENV_COLLISION_CATALOG_RECORD Record{};
			if (CEnv_CollisionCatalog::Try_Find(pDesc->wstrObjectName, &Record))
			{
				Collision.bHasCollMesh = true;
				Collision.bCookCollMesh = true;
				Collision.bUseCollMesh = false;

				Collision.bHasDecorCollisionApxbin = true;
				Collision.strDecorCollisionApxbinName = Record.strApxbinName;
				Collision.strDecorCollisionBfresPath = Record.strBfresPath;
				Collision.eColliderKind = ENV_COLLIDER_KIND::MODEL_MESH;
			}
			else
			{
				Collision.bHasCollMesh = false;
				Collision.bCookCollMesh = false;
				Collision.bUseCollMesh = false;
				Collision.eColliderKind = ENV_COLLIDER_KIND::NONE;
			}

			return;
		}

		// Toy_Obj는 별도 상호작용/기믹 계열이므로 기존 MapCollType 판단을 유지한다.
		if (pDesc->eSourceType == ENV_SOURCE_TYPE::TOY_OBJ)
		{
			if (Collision.bInvalidCollision)
				return;

			if (!Collision.strMapCollType.empty()
				&& !Is_SameText(Collision.strMapCollType, L"None"))
			{
				Collision.eColliderKind = ENV_COLLIDER_KIND::UNKNOWN;
				return;
			}

			Collision.eColliderKind = ENV_COLLIDER_KIND::NONE;
			return;
		}
	}
}

HRESULT CMap_Parser::Parse_Manifest(const _wstring& strManifestPath, MAP_MANIFEST_DESC* pOutManifest)
{
	if (nullptr == pOutManifest)
		return E_FAIL;

	*pOutManifest = {};

	string strContent;
	if (FAILED(CDataLoader::Read_Json(strManifestPath.c_str(), &strContent)))
		return E_FAIL;

	json jRoot;
	try
	{
		jRoot = json::parse(strContent);
	}
	catch (const std::exception& e)
	{
		Log_GameContentWarning(
			"Map manifest parse failed: " + WstrToStr(strManifestPath)
			+ " reason=" + e.what());
		return E_FAIL;
	}

	return Parse_ManifestRoot(jRoot, filesystem::path(strManifestPath), pOutManifest);
}

HRESULT CMap_Parser::Parse_ManifestRoot(const json& jRoot, const filesystem::path& ManifestPath, MAP_MANIFEST_DESC* pOutManifest)
{
	if (nullptr == pOutManifest)
		return E_FAIL;

	*pOutManifest = {};

	const json* pMap = Find_JsonValue(jRoot, "Map");
	const json& jManifest = (nullptr != pMap && pMap->is_object()) ? *pMap : jRoot;

	if (!Try_ReadString(jManifest, "LevelName", &pOutManifest->strLevelName))
		Try_ReadString(jManifest, "Level", &pOutManifest->strLevelName);

	if (pOutManifest->strLevelName.empty())
		pOutManifest->strLevelName = ManifestPath.stem().wstring();

	if (!Try_ReadString(jManifest, "StageFolderName", &pOutManifest->strStageFolderName))
		Try_ReadString(jManifest, "StageFolder", &pOutManifest->strStageFolderName);

	if (pOutManifest->strStageFolderName.empty())
		return E_FAIL;

	if (!Try_ReadString(jManifest, "StageName", &pOutManifest->strStageName))
		pOutManifest->strStageName = pOutManifest->strStageFolderName + L"_MapStage";

	if (FAILED(Parse_Sections(jManifest, pOutManifest)))
		return E_FAIL;

	vector<_wstring> RawEnvJsonPaths;
	if (Try_ReadStringArray(jManifest, "EnvJsonPaths", &RawEnvJsonPaths)
		|| Try_ReadStringArray(jManifest, "EnvJsons", &RawEnvJsonPaths))
	{
		pOutManifest->EnvJsonPaths.clear();
		pOutManifest->EnvJsonPaths.reserve(RawEnvJsonPaths.size());

		for (const _wstring& strRawPath : RawEnvJsonPaths)
			pOutManifest->EnvJsonPaths.push_back(Resolve_PathFromManifest(ManifestPath, strRawPath));
	}

	vector<_wstring> RawLevelDesignJsonPaths;
	if (Try_ReadStringArray(jManifest, "LevelDesignJsonPaths", &RawLevelDesignJsonPaths)
		|| Try_ReadStringArray(jManifest, "LevelDesignJsons", &RawLevelDesignJsonPaths)
		|| Try_ReadStringArray(jManifest, "LevelDesignPaths", &RawLevelDesignJsonPaths))
	{
		pOutManifest->LevelDesignJsonPaths.clear();
		pOutManifest->LevelDesignJsonPaths.reserve(RawLevelDesignJsonPaths.size());

		for (const _wstring& strRawPath : RawLevelDesignJsonPaths)
			pOutManifest->LevelDesignJsonPaths.push_back(Resolve_PathFromManifest(ManifestPath, strRawPath));
	}
	else
	{
		_wstring strLevelDesignPath;
		if (Try_ReadString(jManifest, "LevelDesignPath", &strLevelDesignPath)
			|| Try_ReadString(jManifest, "LevelDesignJsonPath", &strLevelDesignPath)
			|| Try_ReadString(jManifest, "LevelDesign", &strLevelDesignPath))
		{
			pOutManifest->LevelDesignJsonPaths.push_back(
				Resolve_PathFromManifest(ManifestPath, strLevelDesignPath));
		}
	}

	_wstring strDeltaPath;
	if (Try_ReadString(jManifest, "DeltaPath", &strDeltaPath)
		|| Try_ReadString(jManifest, "OverridePath", &strDeltaPath))
	{
		pOutManifest->strDeltaPath = Resolve_PathFromManifest(ManifestPath, strDeltaPath);
	}

	_wstring strCatalogPath;
	if (Try_ReadString(jManifest, "DecorCollisionCatalog", &strCatalogPath)
		|| Try_ReadString(jManifest, "EnvCollisionCatalog", &strCatalogPath)
		|| Try_ReadString(jManifest, "CollisionCatalog", &strCatalogPath))
	{
		pOutManifest->strDecorCollisionCatalogPath =
			Resolve_PathFromManifest(ManifestPath, strCatalogPath);
	}
	else
	{
		// Manifest가 Resources/Map/StageX/StageX_Manifest.json 아래에 있다는 전제의 기본값.
		pOutManifest->strDecorCollisionCatalogPath =
			(ManifestPath.parent_path().parent_path() / L"DecorCollisionCatalog.json")
			.lexically_normal()
			.wstring();
	}

	return S_OK;
}

HRESULT CMap_Parser::Parse_Sections(
	const json& jManifest,
	MAP_MANIFEST_DESC* pOutManifest)
{
	if (nullptr == pOutManifest)
		return E_FAIL;

	pOutManifest->SectionNames.clear();
	pOutManifest->SectionTypes.clear();
	pOutManifest->SectionRenderIDs.clear();

	const json* pSections = Find_JsonValue(jManifest, "Sections");
	if (nullptr != pSections)
	{
		if (!pSections->is_array())
			return E_FAIL;

		for (const json& jSection : *pSections)
		{
			if (FAILED(Parse_SectionEntry(jSection, pOutManifest)))
				return E_FAIL;
		}

		return S_OK;
	}

	vector<_wstring> SectionNames;
	if (!Try_ReadStringArray(jManifest, "SectionNames", &SectionNames))
		return E_FAIL;

	const json* pSectionTypes = Find_JsonValue(jManifest, "SectionTypes");
	if (nullptr == pSectionTypes || !pSectionTypes->is_array() || pSectionTypes->size() !=
		SectionNames.size())
		return E_FAIL;

	const json* pSectionRenderIDs = Find_JsonValue(jManifest, "SectionRenderIDs");
	if (nullptr != pSectionRenderIDs && (!pSectionRenderIDs->is_array() || pSectionRenderIDs->size() !=
		SectionNames.size()))
		return E_FAIL;

	pOutManifest->SectionNames.reserve(SectionNames.size());
	pOutManifest->SectionTypes.reserve(SectionNames.size());
	pOutManifest->SectionRenderIDs.reserve(SectionNames.size());

	for (size_t i = 0; i < SectionNames.size(); ++i)
	{
		MAP_SECTION_TYPE eSectionType = MAP_SECTION_TYPE::UNKNOWN;
		if (!Try_ParseSectionType((*pSectionTypes)[i], &eSectionType))
			return E_FAIL;

		RENDERID eRenderID = RENDERID::NONBLEND;
		if (nullptr != pSectionRenderIDs && !Try_ParseRenderID((*pSectionRenderIDs)[i], &eRenderID))
			return E_FAIL;

		pOutManifest->SectionNames.push_back(SectionNames[i]);
		pOutManifest->SectionTypes.push_back(eSectionType);
		pOutManifest->SectionRenderIDs.push_back(eRenderID);
	}

	return S_OK;
}

HRESULT CMap_Parser::Parse_SectionEntry(
	const json& jSection,
	MAP_MANIFEST_DESC* pOutManifest)
{
	if (nullptr == pOutManifest || !jSection.is_object())
		return E_FAIL;

	_wstring strSectionName;
	if (!Try_ReadString(jSection, "Name", &strSectionName)
		&& !Try_ReadString(jSection, "SectionName", &strSectionName))
	{
		return E_FAIL;
	}

	const json* pType = Find_JsonValue(jSection, "Type");
	if (nullptr == pType)
		pType = Find_JsonValue(jSection, "SectionType");

	MAP_SECTION_TYPE eSectionType = MAP_SECTION_TYPE::UNKNOWN;
	if (nullptr == pType || !Try_ParseSectionType(*pType, &eSectionType))
		return E_FAIL;

	RENDERID eRenderID = RENDERID::NONBLEND;
	const json* pRenderID = Find_JsonValue(jSection, "RenderID");
	if (nullptr == pRenderID)
		pRenderID = Find_JsonValue(jSection, "RenderGroup");

	if (nullptr != pRenderID && !Try_ParseRenderID(*pRenderID, &eRenderID))
		return E_FAIL;

	pOutManifest->SectionNames.push_back(strSectionName);
	pOutManifest->SectionTypes.push_back(eSectionType);
	pOutManifest->SectionRenderIDs.push_back(eRenderID);

	return S_OK;
}

_bool CMap_Parser::Try_ReadStringArray(
	const json& jSource,
	const string& strPath,
	vector<_wstring>* pOut)
{
	if (nullptr == pOut)
		return false;

	const json* pValue = Find_JsonValue(jSource, strPath);
	if (nullptr == pValue || !pValue->is_array())
		return false;

	pOut->clear();
	pOut->reserve(pValue->size());

	for (const json& jItem : *pValue)
	{
		if (!jItem.is_string())
			return false;

		pOut->push_back(StrToWstr(jItem.get<string>()));
	}

	return true;
}

_bool CMap_Parser::Try_ParseSectionType(const json& jValue, MAP_SECTION_TYPE* pOut)
{
	if (nullptr == pOut)
		return false;

	if (jValue.is_number_integer())
	{
		const int iValue = jValue.get<int>();
		if (0 <= iValue && iValue < static_cast<int>(MAP_SECTION_TYPE::END))
		{
			*pOut = static_cast<MAP_SECTION_TYPE>(iValue);
			return true;
		}

		return false;
	}

	if (!jValue.is_string())
		return false;

	const string strValue = To_UpperCopy(jValue.get<string>());

	if (strValue == "DEFAULT") { *pOut = MAP_SECTION_TYPE::DEFAULT; return true; }
	if (strValue == "BUILDING") { *pOut = MAP_SECTION_TYPE::BUILDING; return true; }
	if (strValue == "GROUND") { *pOut = MAP_SECTION_TYPE::GROUND; return true; }
	if (strValue == "ROCK") { *pOut = MAP_SECTION_TYPE::ROCK; return true; }
	if (strValue == "TRANSPARENT") { *pOut = MAP_SECTION_TYPE::TRANSPARENT; return true; }
	if (strValue == "WATER") { *pOut = MAP_SECTION_TYPE::WATER; return true; }
	if (strValue == "EFFECT") { *pOut = MAP_SECTION_TYPE::EFFECT; return true; }
	if (strValue == "UNKNOWN") { *pOut = MAP_SECTION_TYPE::UNKNOWN; return true; }

	return false;
}

_bool CMap_Parser::Try_ParseRenderID(const json& jValue, RENDERID* pOut)
{
	if (nullptr == pOut)
		return false;

	if (jValue.is_number_integer())
	{
		const int iValue = jValue.get<int>();
		if (0 <= iValue && iValue < static_cast<int>(RENDERID::END))
		{
			*pOut = static_cast<RENDERID>(iValue);
			return true;
		}

		return false;
	}

	if (!jValue.is_string())
		return false;

	const string strValue = To_UpperCopy(jValue.get<string>());

	if (strValue == "PRIORITY") { *pOut = RENDERID::PRIORITY; return true; }
	if (strValue == "ANIM_SHADOW") { *pOut = RENDERID::SHADOW; return true; }
	if (strValue == "NONBLEND") { *pOut = RENDERID::NONBLEND; return true; }
	if (strValue == "NONLIGHT") { *pOut = RENDERID::NONLIGHT; return true; }
	if (strValue == "BLEND") { *pOut = RENDERID::BLEND; return true; }

	return false;
}

_wstring CMap_Parser::Resolve_PathFromManifest(
	const filesystem::path& ManifestPath,
	const _wstring& strRawPath)
{
	filesystem::path RawPath(strRawPath);
	if (RawPath.is_absolute())
		return RawPath.lexically_normal().wstring();

	const filesystem::path BaseDir = ManifestPath.parent_path();
	return (BaseDir / RawPath).lexically_normal().wstring();
}

HRESULT CMap_Parser::Parse_EnvJson(const wstring& strJsonPath, vector<ENV_OBJECT_DESC>* pOutDescs)
{
	if (nullptr == pOutDescs)
		return E_FAIL;

	pOutDescs->clear();

	string strContent;
	if (FAILED(CDataLoader::Read_Json(strJsonPath.c_str(), &strContent)))
		return E_FAIL;

	json jRoot;
	try
	{
		jRoot = json::parse(strContent);
	}
	catch (const std::exception& e)
	{
		Log_GameContentWarning(
			"EnvObject json parse failed: " + WstrToStr(strJsonPath)
			+ " reason=" + e.what());
		return E_FAIL;
	}

	return Parse_EnvRoot(jRoot, pOutDescs);
}

HRESULT CMap_Parser::Parse_EnvRoot(const json& jRoot, vector<ENV_OBJECT_DESC>* pOutDescs)
{
	if (nullptr == pOutDescs)
		return E_FAIL;

	wstring wstrSourceFile;
	Try_ReadString(jRoot, "source_file", &wstrSourceFile);

	if (Is_SkippedEnvSourceFile(wstrSourceFile))
		return S_OK;

	const json* pData = Find_JsonValue(jRoot, "data");
	if (nullptr == pData || !pData->is_object())
		return E_FAIL;

	for (auto Iter = pData->begin(); Iter != pData->end(); ++Iter)
	{
		Parse_SectionObject(
			wstrSourceFile,
			StrToWstr(Iter.key()),
			Iter.value(),
			pOutDescs);
	}

	return S_OK;
}

void CMap_Parser::Parse_SectionObject(
	const wstring& wstrSourceFile,
	const wstring& wstrSection,
	const json& jSection,
	vector<ENV_OBJECT_DESC>* pOutDescs)
{
	if (!jSection.is_object() || nullptr == pOutDescs)
		return;

	const ENV_SOURCE_TYPE eSourceType = Classify_SourceType(wstrSourceFile);

	for (auto Iter = jSection.begin(); Iter != jSection.end(); ++Iter)
	{
		if (!Iter.value().is_object())
			continue;

		const wstring wstrEntryKey = StrToWstr(Iter.key());

		switch (eSourceType)
		{
		case ENV_SOURCE_TYPE::DECOR_DECOR:
			Parse_DecorEntry(
				wstrSourceFile,
				wstrSection,
				wstrEntryKey,
				Iter.value(),
				pOutDescs);
			break;

		case ENV_SOURCE_TYPE::TOY_OBJ:
			if (0 == _wcsicmp(wstrSection.c_str(), L"Standard"))
			{
				Parse_ToyObjEntry(
					wstrSourceFile,
					wstrSection,
					wstrEntryKey,
					Iter.value(),
					pOutDescs);
			}
			break;

		case ENV_SOURCE_TYPE::DECOR_OBJ:
			Parse_EffectEntry(
				wstrSourceFile,
				wstrSection,
				wstrEntryKey,
				Iter.value(),
				pOutDescs);
			break;

		default:
			break;
		}
	}
}

void CMap_Parser::Parse_DecorEntry(const wstring& wstrSourceFile, const wstring& wstrSection, const wstring& wstrEntryKey,
	const json& jEntry, vector<ENV_OBJECT_DESC>* pOutDescs)
{
	if (nullptr == pOutDescs)
		return;

	ENV_OBJECT_DESC Desc = Make_BaseDesc(wstrSourceFile, wstrSection, wstrEntryKey);
	Desc.eKind = ENV_OBJECT_KIND::STATIC;
	Desc.jRawProperties = jEntry;

	if (!Try_ReadString(jEntry, "Basic.ObjectName", &Desc.wstrObjectName))
		return;

	Fill_CommonFlags(jEntry, &Desc);
	Desc.tRender.bIsDecal = Is_SameText(Desc.tRender.strLayerName, L"Decal");

	Try_BuildWorldMatrixFromArray(jEntry, "WorldMtx", &Desc);

	Resolve_EnvColliderKind(&Desc);

	pOutDescs->push_back(Desc);
}

void CMap_Parser::Parse_ToyObjEntry(
	const wstring& wstrSourceFile,
	const wstring& wstrSection,
	const wstring& wstrEntryKey,
	const json& jEntry,
	vector<ENV_OBJECT_DESC>* pOutDescs)
{
	if (nullptr == pOutDescs)
		return;

	ENV_OBJECT_DESC Desc = Make_BaseDesc(wstrSourceFile, wstrSection, wstrEntryKey);
	CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(Desc);

	Desc.eKind = ENV_OBJECT_KIND::INTERACT;
	Desc.jRawProperties = jEntry;

	if (!Try_ReadString(jEntry, "Basic.ObjectName", &Desc.wstrObjectName))
		return;

	Fill_CommonFlags(jEntry, &Desc);
	Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.Position", &Desc.vPosition);
	Try_ReadFloat4Array(jEntry, "Basic.BasicInfo.Rotation", &Desc.vRotation);
	Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.SceneScale", &Desc.vScale);

	BaseDesc.vPosition = _float4(Desc.vPosition.x, Desc.vPosition.y, Desc.vPosition.z, 1.f);

	Try_ReadString(
		jEntry,
		"Gimmick.InteractiveDecorParts.MainComponent.MapCollType",
		&Desc.tCollision.strMapCollType);
	Try_ReadFloat(
		jEntry,
		"Gimmick.InteractiveDecorParts.MainComponent.MapCollRadius",
		&Desc.tCollision.fMapCollRadius);
	Try_ReadFloat3Array(
		jEntry,
		"Gimmick.InteractiveDecorParts.MainComponent.Size",
		&Desc.tCollision.vSize);

	Resolve_EnvColliderKind(&Desc);

	pOutDescs->push_back(Desc);
}

void CMap_Parser::Parse_EffectEntry(const _wstring& wstrSourceFile, const _wstring& wstrSection, const _wstring& wstrEntryKey,
	const json& jEntry, vector<ENV_OBJECT_DESC>* pOutDescs)
{
	if (nullptr == pOutDescs)
		return;

	ENV_OBJECT_DESC Desc = Make_BaseDesc(wstrSourceFile, wstrSection, wstrEntryKey);
	CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(Desc);

	Desc.eKind = ENV_OBJECT_KIND::EFFECT;
	Desc.jRawProperties = jEntry;

	Try_ReadString(jEntry, "Basic.ObjectName", &Desc.wstrObjectName);
	Try_ReadString(jEntry, "Basic.BasicInfo.ObjectName", &Desc.wstrObjectName);
	Try_ReadString(jEntry, "ComponentName", &Desc.wstrComponentName);

	Fill_CommonFlags(jEntry, &Desc);

	Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.Position", &Desc.vPosition);
	Try_ReadFloat4Array(jEntry, "Basic.BasicInfo.Rotation", &Desc.vRotation);
	Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.SceneScale", &Desc.vScale);

	BaseDesc.vPosition = _float4(Desc.vPosition.x, Desc.vPosition.y, Desc.vPosition.z, 1.f);

	Try_ReadFloat3Array(jEntry, "Position", &Desc.tEffect.vPosition);
	Try_ReadFloat3Array(jEntry, "Direction", &Desc.tEffect.vDirection);
	Try_ReadFloat4Array(jEntry, "Color", &Desc.tEffect.vColor);
	Try_ReadFloat(jEntry, "Intensity", &Desc.tEffect.fIntensity);
	Try_ReadFloat(jEntry, "Range", &Desc.tEffect.fRange);
	Try_ReadFloat(jEntry, "Angle", &Desc.tEffect.fAngle);
	Try_ReadFloat(jEntry, "DecayStartAngle", &Desc.tEffect.fDecayStartAngle);
	Try_ReadFloat3Array(jEntry, "AreaCenter", &Desc.tEffect.vAreaCenter);
	Try_ReadFloat3Array(jEntry, "AreaSize", &Desc.tEffect.vAreaSize);
	Try_ReadFloat4Array(jEntry, "AreaRot", &Desc.tEffect.vAreaRot);
	Try_ReadFloat3Array(jEntry, "EmitPos", &Desc.tEffect.vEmitPos);
	Try_ReadString(jEntry, "AreaLightName", &Desc.tEffect.strAreaLightName);
	Try_ReadString(jEntry, "ActivationCondition", &Desc.tEffect.strActivationCondition);
	Try_ReadString(jEntry, "HideKind", &Desc.tEffect.strHideKind);
	Try_ReadFloat(jEntry, "ExposureValue", &Desc.tEffect.fExposureValue);
	Try_ReadFloat(jEntry, "TransitionSec", &Desc.tEffect.fTransitionSec);
	Try_ReadFloat(jEntry, "InTransitionSec", &Desc.tEffect.fInTransitionSec);
	Try_ReadFloat(jEntry, "OutTransitionSec", &Desc.tEffect.fOutTransitionSec);
	Try_ReadString(jEntry, "Kind", &Desc.tEffect.strKind);

	Try_BuildWorldMatrixFromArray(jEntry, "WorldMtx", &Desc);

	const json* pMainComponent = nullptr;

	if (const json* p = Find_JsonValue(jEntry, "Gimmick.LocalAreaLight.MainComponent"))
	{
		pMainComponent = p;
		Desc.wstrComponentName = L"LocalAreaLight";
	}
	else if (const json* p = Find_JsonValue(jEntry, "Gimmick.DecorPartsCullingArea.MainComponent"))
	{
		pMainComponent = p;
		Desc.wstrComponentName = L"DecorPartsCullingArea";
	}
	else if (const json* p = Find_JsonValue(jEntry, "Gimmick.ToneMappingArea.MainComponent"))
	{
		pMainComponent = p;
		Desc.wstrComponentName = L"ToneMappingArea";
	}
	else if (const json* p = Find_JsonValue(jEntry, "Gimmick.FieldEffect.MainComponent"))
	{
		pMainComponent = p;
		Desc.wstrComponentName = L"FieldEffect";
	}

	if (nullptr != pMainComponent)
	{
		Try_ReadFloat(*pMainComponent, "TransitionSec", &Desc.tEffect.fTransitionSec);
		Try_ReadFloat(*pMainComponent, "InTransitionSec", &Desc.tEffect.fInTransitionSec);
		Try_ReadFloat(*pMainComponent, "OutTransitionSec", &Desc.tEffect.fOutTransitionSec);
	}

	if (Desc.wstrComponentName == L"ToneMappingArea")
	{
		Try_ReadFloat3Array(*pMainComponent, "Size", &Desc.tEffect.vAreaSize);
		Try_ReadFloat(*pMainComponent, "ExposureValue", &Desc.tEffect.fExposureValue);

		Desc.tEffect.vAreaCenter = { 0.f, 0.f, 0.f };
		Desc.tEffect.vAreaRot = Desc.vRotation;
		Desc.vScale = { 1.f, 1.f, 1.f };
	}

	Desc.tEffect.eEffectType = Classify_EffectType(Desc.wstrObjectName, Desc.wstrComponentName);

	Resolve_EnvColliderKind(&Desc);

	pOutDescs->push_back(Desc);
}

ENV_OBJECT_DESC CMap_Parser::Make_BaseDesc(const wstring& wstrSourceFile, const wstring& wstrSection, const wstring& wstrEntryKey)
{
	ENV_OBJECT_DESC Desc{};
	Desc.wstrSourceFile = wstrSourceFile;
	Desc.wstrSection = wstrSection;
	Desc.wstrEntryKey = wstrEntryKey;
	Desc.eSourceType = Classify_SourceType(wstrSourceFile);
	return Desc;
}

ENV_SOURCE_TYPE CMap_Parser::Classify_SourceType(const wstring& wstrSourceFile)
{
	if (0 == _wcsicmp(wstrSourceFile.c_str(), L"Decor_Decor.bin"))
		return ENV_SOURCE_TYPE::DECOR_DECOR;
	if (0 == _wcsicmp(wstrSourceFile.c_str(), L"Toy_Obj.bin"))
		return ENV_SOURCE_TYPE::TOY_OBJ;
	if (0 == _wcsicmp(wstrSourceFile.c_str(), L"Decor_Obj.bin"))
		return ENV_SOURCE_TYPE::DECOR_OBJ;

	return ENV_SOURCE_TYPE::UNKNOWN;
}

ENV_EFFECT_TYPE CMap_Parser::Classify_EffectType(
	const wstring& wstrObjectName,
	const wstring& wstrComponentName)
{
	const wstring Combined = wstrObjectName + L" " + wstrComponentName;

	if (wstring::npos != Combined.find(L"LocalAreaLight"))
		return ENV_EFFECT_TYPE::LOCAL_AREA_LIGHT;
	if (wstring::npos != Combined.find(L"ToneMappingArea"))
		return ENV_EFFECT_TYPE::TONE_MAPPING_AREA;
	if (wstring::npos != Combined.find(L"DecorPartsCullingArea"))
		return ENV_EFFECT_TYPE::DECOR_PARTS_CULLING_AREA;
	if (wstring::npos != Combined.find(L"GrassWind"))
		return ENV_EFFECT_TYPE::GRASS_WIND;
	if (wstring::npos != Combined.find(L"FieldEffect"))
		return ENV_EFFECT_TYPE::FIELD_EFFECT;
	if (wstring::npos != Combined.find(L"FlowerWing"))
		return ENV_EFFECT_TYPE::FLOWER_WING;
	if (wstring::npos != Combined.find(L"SpotLight"))
		return ENV_EFFECT_TYPE::SPOT_LIGHT;

	return ENV_EFFECT_TYPE::UNKNOWN;
}

const json* CMap_Parser::Find_JsonValue(const json& jSource, const string& strPath)
{
	auto ExactIter = jSource.find(strPath);
	if (ExactIter != jSource.end())
		return &(*ExactIter);

	const vector<string> Parts = Split_DottedPath(strPath);

	function<const json* (const json&, size_t)> FindRecursive;
	FindRecursive = [&](const json& Current, size_t iStart) -> const json*
		{
			if (iStart >= Parts.size())
				return &Current;

			if (!Current.is_object())
				return nullptr;

			for (size_t iEnd = Parts.size(); iEnd > iStart; --iEnd)
			{
				string strKey;

				for (size_t i = iStart; i < iEnd; ++i)
				{
					if (i > iStart)
						strKey += ".";

					strKey += Parts[i];
				}

				auto Iter = Current.find(strKey);
				if (Iter == Current.end())
					continue;

				if (const json* pFound = FindRecursive(*Iter, iEnd))
					return pFound;
			}

			return nullptr;
		};

	return FindRecursive(jSource, 0);
}

_bool CMap_Parser::Try_ReadString(const json& jSource, const string& strPath, wstring* pOut)
{
	if (nullptr == pOut)
		return false;

	const json* pValue = Find_JsonValue(jSource, strPath);
	if (nullptr == pValue || !pValue->is_string())
		return false;

	*pOut = StrToWstr(pValue->get<string>());
	return true;
}

_bool CMap_Parser::Try_ReadUInt(const json& jSource, const string& strPath, _uint* pOut)
{
	if (nullptr == pOut)
		return false;

	const json* pValue = Find_JsonValue(jSource, strPath);
	if (nullptr == pValue || !pValue->is_number_unsigned())
		return false;

	*pOut = pValue->get<_uint>();
	return true;
}

_bool CMap_Parser::Try_ReadFloat(const json& jSource, const string& strPath, _float* pOut)
{
	if (nullptr == pOut)
		return false;

	const json* pValue = Find_JsonValue(jSource, strPath);
	if (nullptr == pValue || !pValue->is_number())
		return false;

	*pOut = pValue->get<_float>();
	return true;
}

_bool CMap_Parser::Try_ReadFloat3Array(const json& jSource, const string& strPath, _float3* pOut)
{
	if (nullptr == pOut)
		return false;

	const json* pValue = Find_JsonValue(jSource, strPath);
	if (nullptr == pValue || !pValue->is_array() || pValue->size() < 3)
		return false;

	pOut->x = (*pValue)[0].get<_float>();
	pOut->y = (*pValue)[1].get<_float>();
	pOut->z = (*pValue)[2].get<_float>();
	return true;
}

_bool CMap_Parser::Try_ReadFloat4Array(const json& jSource, const string& strPath, _float4* pOut)
{
	if (nullptr == pOut)
		return false;

	const json* pValue = Find_JsonValue(jSource, strPath);
	if (nullptr == pValue || !pValue->is_array() || pValue->size() < 4)
		return false;

	pOut->x = (*pValue)[0].get<_float>();
	pOut->y = (*pValue)[1].get<_float>();
	pOut->z = (*pValue)[2].get<_float>();
	pOut->w = (*pValue)[3].get<_float>();
	return true;
}

_bool CMap_Parser::Try_ReadBoolFromNumeric(const json& jSource, const string& strPath, _bool* pOut)
{
	_uint iValue = 0;
	if (!Try_ReadUInt(jSource, strPath, &iValue))
		return false;

	*pOut = (0 != iValue);
	return true;
}

_bool CMap_Parser::Try_BuildWorldMatrixFromArray(
	const json& jSource,
	const string& strPath,
	ENV_OBJECT_DESC* pOutDesc)
{
	if (nullptr == pOutDesc)
		return false;

	const json* pValue = Find_JsonValue(jSource, strPath);
	if (nullptr == pValue || !pValue->is_array() || pValue->size() < 12)
		return false;

	_float4x4 Mat = {};
	Mat.m[0][0] = (*pValue)[0].get<_float>();
	Mat.m[0][1] = (*pValue)[1].get<_float>();
	Mat.m[0][2] = (*pValue)[2].get<_float>();
	Mat.m[1][0] = (*pValue)[3].get<_float>();
	Mat.m[1][1] = (*pValue)[4].get<_float>();
	Mat.m[1][2] = (*pValue)[5].get<_float>();
	Mat.m[2][0] = (*pValue)[6].get<_float>();
	Mat.m[2][1] = (*pValue)[7].get<_float>();
	Mat.m[2][2] = (*pValue)[8].get<_float>();
	Mat.m[3][0] = (*pValue)[9].get<_float>();
	Mat.m[3][1] = (*pValue)[10].get<_float>();
	Mat.m[3][2] = (*pValue)[11].get<_float>();
	Mat.m[3][3] = 1.f;

	CGameObject::GAMEOBJECT_DESC& BaseDesc =
		static_cast<CGameObject::GAMEOBJECT_DESC&>(*pOutDesc);

	pOutDesc->matWorld = Mat;
	pOutDesc->bHasWorldMatrix = true;
	BaseDesc.vRight = _float4(Mat.m[0][0], Mat.m[0][1], Mat.m[0][2], 0.f);
	BaseDesc.vUp = _float4(Mat.m[1][0], Mat.m[1][1], Mat.m[1][2], 0.f);
	BaseDesc.vLook = _float4(Mat.m[2][0], Mat.m[2][1], Mat.m[2][2], 0.f);
	BaseDesc.vPosition = _float4(Mat.m[3][0], Mat.m[3][1], Mat.m[3][2], 1.f);
	pOutDesc->vPosition = _float3(Mat.m[3][0], Mat.m[3][1], Mat.m[3][2]);

	return true;
}

void CMap_Parser::Fill_CommonFlags(const json& jEntry, ENV_OBJECT_DESC* pDesc)
{
	if (nullptr == pDesc)
		return;

	if (!Try_ReadBoolFromNumeric(jEntry, "IsInvalidCollision", &pDesc->tCollision.bInvalidCollision))
		Try_ReadBoolFromNumeric(jEntry, "Basic.Model.IsInvalidCollision", &pDesc->tCollision.bInvalidCollision);

	Try_ReadBoolFromNumeric(jEntry, "IsInvisibleCollision", &pDesc->tCollision.bInvisibleCollision);

	if (!Try_ReadBoolFromNumeric(jEntry, "IsSlipFallCollision", &pDesc->tCollision.bSlipFallCollision))
		Try_ReadBoolFromNumeric(jEntry, "Basic.Model.IsSlipFallCollisionScene", &pDesc->tCollision.bSlipFallCollision);

	Try_ReadBoolFromNumeric(jEntry, "IsUseObjCollReaction", &pDesc->tCollision.bUseObjCollisionReaction);
	Try_ReadBoolFromNumeric(jEntry, "IsNeedUpdateCollisionByAnim", &pDesc->tCollision.bNeedUpdateCollisionByAnim);
	Try_ReadBoolFromNumeric(jEntry, "IsOverrideCollisionAttr", &pDesc->tCollision.bOverrideCollisionAttr);

	Try_ReadString(jEntry, "OverrideCollisionType", &pDesc->tCollision.strOverrideCollisionType);
	Try_ReadString(jEntry, "OverrideCollisionTypeInside", &pDesc->tCollision.strOverrideCollisionTypeInside);

	Try_ReadBoolFromNumeric(jEntry, "IsShadowMappingCaster", &pDesc->tRender.bHasShadow);
	// Source capability defaults to active shadow usage. Map edit override may disable it later.
	pDesc->tRender.bUseShadow = pDesc->tRender.bHasShadow;
	pDesc->tRender.bShadowMappingCaster = pDesc->tRender.bUseShadow; // Legacy transitional.

	if (!Try_ReadBoolFromNumeric(jEntry, "UseLodCulling", &pDesc->tRender.bUseCullDistance))
		Try_ReadBoolFromNumeric(jEntry, "Basic.Model.UseLodCulling", &pDesc->tRender.bUseCullDistance);

	if (!Try_ReadBoolFromNumeric(jEntry, "UseNearDistAlpha", &pDesc->tRender.bUseNearDistAlpha))
		Try_ReadBoolFromNumeric(jEntry, "Basic.Model.UseNearDistAlpha", &pDesc->tRender.bUseNearDistAlpha);

	if (!Try_ReadFloat(jEntry, "NearDistAlphaLengthRate", &pDesc->tRender.fNearDistAlphaLengthRate))
		Try_ReadFloat(jEntry, "Basic.Model.NearDistAlphaLengthRate", &pDesc->tRender.fNearDistAlphaLengthRate);

	Try_ReadString(jEntry, "Decor.LayerName", &pDesc->tRender.strLayerName);
	Try_ReadUInt(jEntry, "HideFlag", &pDesc->tRender.iHideFlag);

	if (!Try_ReadUInt(jEntry, "Uid", &pDesc->iUid))
		Try_ReadUInt(jEntry, "Basic.BasicInfo.Uid", &pDesc->iUid);
}

NS_END