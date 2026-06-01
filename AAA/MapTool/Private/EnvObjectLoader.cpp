#include "EnvObjectLoader.h"

#include <filesystem>
#include <unordered_set>

#include "EnvObject_Effect.h"
#include "EnvObject_Interact.h"
#include "EnvObject_Static.h"
#include "DataLoader.h"
#include "GameInstance_Proxy.h"
#include "MapTool_Func.h"
#include "Model.h"

NS_BEGIN(Client)

namespace
{
	using namespace std::filesystem;

	constexpr wchar_t kLayerEnvStatic[] = L"Layer_EnvStatic";
	constexpr wchar_t kLayerEnvInteract[] = L"Layer_EnvInteract";
	constexpr wchar_t kLayerEnvEffect[] = L"Layer_EnvEffect";
	constexpr wchar_t kModelRoot[] = L"../../Resources/Models/Test";

	unordered_map<wstring, wstring>& Get_ModelPathCache()
	{
		static unordered_map<wstring, wstring> s_Cache;
		return s_Cache;
	}

	_bool& Get_ModelPathCacheBuilt()
	{
		static _bool s_bBuilt = false;
		return s_bBuilt;
	}

	unordered_set<wstring>& Get_MissingModelLogSet()
	{
		static unordered_set<wstring> s_Set;
		return s_Set;
	}

	vector<string> Split_DottedPath(const string& strPath)
	{
		vector<string> Parts;
		size_t iStart = 0;
		while (iStart < strPath.size())
		{
			size_t iDot = strPath.find('.', iStart);
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

	const json* Find_JsonValue(const json& jSource, const string& strPath)
	{
		auto ExactIter = jSource.find(strPath);
		if (ExactIter != jSource.end())
			return &(*ExactIter);

		const json* pCurrent = &jSource;
		for (const string& Part : Split_DottedPath(strPath))
		{
			if (!pCurrent->is_object())
				return nullptr;

			auto Iter = pCurrent->find(Part);
			if (Iter == pCurrent->end())
				return nullptr;

			pCurrent = &(*Iter);
		}

		return pCurrent;
	}

	_bool Try_ReadString(const json& jSource, const string& strPath, wstring* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_string())
			return false;

		*pOut = StrToWstr(pValue->get<string>());
		return true;
	}

	_bool Try_ReadUInt(const json& jSource, const string& strPath, _uint* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_number_unsigned())
			return false;

		*pOut = pValue->get<_uint>();
		return true;
	}

	_bool Try_ReadFloat(const json& jSource, const string& strPath, _float* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_number())
			return false;

		*pOut = pValue->get<_float>();
		return true;
	}

	_bool Try_ReadBoolFromNumeric(const json& jSource, const string& strPath, _bool* pOut)
	{
		_uint iValue = 0;
		if (!Try_ReadUInt(jSource, strPath, &iValue))
			return false;

		*pOut = (0 != iValue);
		return true;
	}

	_bool Try_ReadFloat3Array(const json& jSource, const string& strPath, _float3* pOut)
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

	_bool Try_ReadFloat4Array(const json& jSource, const string& strPath, _float4* pOut)
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

	_bool Try_BuildWorldMatrixFromArray(const json& jSource, const string& strPath, ENV_OBJECT_DESC* pOutDesc)
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

		CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(*pOutDesc);
		pOutDesc->matWorld = Mat;
		pOutDesc->bHasWorldMatrix = true;
		BaseDesc.vRight = _float4(Mat.m[0][0], Mat.m[0][1], Mat.m[0][2], 0.f);
		BaseDesc.vUp = _float4(Mat.m[1][0], Mat.m[1][1], Mat.m[1][2], 0.f);
		BaseDesc.vLook = _float4(Mat.m[2][0], Mat.m[2][1], Mat.m[2][2], 0.f);
		BaseDesc.vPosition = _float4(Mat.m[3][0], Mat.m[3][1], Mat.m[3][2], 1.f);
		pOutDesc->vPosition = _float3(Mat.m[3][0], Mat.m[3][1], Mat.m[3][2]);
		return true;
	}

	void Build_ModelPathCache_IfNeeded()
	{
		if (Get_ModelPathCacheBuilt())
			return;

		Get_ModelPathCacheBuilt() = true;
		error_code ErrorCode;
		const path Root = weakly_canonical(path(kModelRoot), ErrorCode);
		if (ErrorCode || !exists(Root))
		{
			MapTool::Log_Warning("EnvObject model root missing: ../../Resources/Models/Test");
			return;
		}

		for (recursive_directory_iterator Iter(Root, directory_options::skip_permission_denied, ErrorCode), End;
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

			Get_ModelPathCache().try_emplace(FilePath.stem().wstring(), FilePath.wstring());
		}
	}

	ENV_SOURCE_TYPE Classify_SourceType(const wstring& strSourceFile)
	{
		if (0 == _wcsicmp(strSourceFile.c_str(), L"Decor_Decor.bin"))
			return ENV_SOURCE_TYPE::DECOR_DECOR;
		if (0 == _wcsicmp(strSourceFile.c_str(), L"Toy_Decor.bin"))
			return ENV_SOURCE_TYPE::TOY_DECOR;
		if (0 == _wcsicmp(strSourceFile.c_str(), L"Toy_Obj.bin"))
			return ENV_SOURCE_TYPE::TOY_OBJ;
		if (0 == _wcsicmp(strSourceFile.c_str(), L"Decor_Obj.bin"))
			return ENV_SOURCE_TYPE::DECOR_OBJ;
		return ENV_SOURCE_TYPE::UNKNOWN;
	}

	ENV_EFFECT_TYPE Classify_EffectType(const wstring& strObjectName, const wstring& strComponentName)
	{
		const wstring Combined = strObjectName + L" " + strComponentName;
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

	void Fill_CommonFlags(const json& jEntry, ENV_OBJECT_DESC* pDesc)
	{
		if (nullptr == pDesc)
			return;

		Try_ReadBoolFromNumeric(jEntry, "IsInvalidCollision", &pDesc->tCollision.bInvalidCollision);
		Try_ReadBoolFromNumeric(jEntry, "IsInvisibleCollision", &pDesc->tCollision.bInvisibleCollision);
		Try_ReadBoolFromNumeric(jEntry, "IsSlipFallCollision", &pDesc->tCollision.bSlipFallCollision);
		Try_ReadBoolFromNumeric(jEntry, "IsUseObjCollReaction", &pDesc->tCollision.bUseObjCollisionReaction);
		Try_ReadBoolFromNumeric(jEntry, "IsNeedUpdateCollisionByAnim", &pDesc->tCollision.bNeedUpdateCollisionByAnim);
		Try_ReadBoolFromNumeric(jEntry, "IsOverrideCollisionAttr", &pDesc->tCollision.bOverrideCollisionAttr);

		Try_ReadString(jEntry, "OverrideCollisionType", &pDesc->tCollision.strOverrideCollisionType);
		Try_ReadString(jEntry, "OverrideCollisionTypeInside", &pDesc->tCollision.strOverrideCollisionTypeInside);

		Try_ReadBoolFromNumeric(jEntry, "IsShadowMappingCaster", &pDesc->tRender.bShadowMappingCaster);
		Try_ReadBoolFromNumeric(jEntry, "UseLodCulling", &pDesc->tRender.bUseLodCulling);
		Try_ReadBoolFromNumeric(jEntry, "UseNearDistAlpha", &pDesc->tRender.bUseNearDistAlpha);
		Try_ReadFloat(jEntry, "NearDistAlphaLengthRate", &pDesc->tRender.fNearDistAlphaLengthRate);
		Try_ReadString(jEntry, "Decor.LayerName", &pDesc->tRender.strLayerName);
		Try_ReadUInt(jEntry, "HideFlag", &pDesc->tRender.iHideFlag);
		Try_ReadUInt(jEntry, "Uid", &pDesc->iUid);
	}

	_bool Try_ResolveModelForObject(ENV_OBJECT_DESC* pDesc)
	{
		if (nullptr == pDesc || pDesc->strObjectName.empty())
			return false;

		Build_ModelPathCache_IfNeeded();
		const auto Iter = Get_ModelPathCache().find(pDesc->strObjectName);
		if (Iter == Get_ModelPathCache().end())
			return false;

		pDesc->strModelProtoTag = L"Prototype_Component_Model_Env_" + pDesc->strObjectName;
		pDesc->strModelPath = Iter->second;
		return true;
	}

	HRESULT Ensure_ModelPrototype(
		CGameInstance_Proxy* pProxy,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		_uint iModelLevel,
		const ENV_OBJECT_DESC& Desc)
	{
		if (Desc.strModelProtoTag.empty() || Desc.strModelPath.empty())
			return S_FALSE;

		if (pProxy->Has_Prototype(iModelLevel, Desc.strModelProtoTag))
			return S_OK;

		const string strModelPath = WstrToStr(Desc.strModelPath);
		return pProxy->Add_Prototype(
			iModelLevel,
			Desc.strModelProtoTag.c_str(),
			CModel::Create(pDevice, pContext, MODEL::NONANIM, strModelPath.c_str()));
	}

	wstring Make_ObjectName(const ENV_OBJECT_DESC& Desc)
	{
		wstring strName = L"Env_" + Desc.strObjectName;
		if (0 != Desc.iUid)
			strName += L"_" + to_wstring(Desc.iUid);
		else if (!Desc.strEntryKey.empty())
			strName += L"_" + Desc.strEntryKey;
		return strName;
	}

	const wchar_t* Get_ObjectPrototypeTag(ENV_OBJECT_KIND eKind)
	{
		switch (eKind)
		{
		case ENV_OBJECT_KIND::STATIC: return CEnvObject_Static::PROTOTYPE_TAG;
		case ENV_OBJECT_KIND::INTERACT: return CEnvObject_Interact::PROTOTYPE_TAG;
		case ENV_OBJECT_KIND::EFFECT: return CEnvObject_Effect::PROTOTYPE_TAG;
		default: return nullptr;
		}
	}

	const wchar_t* Get_ObjectLayerTag(ENV_OBJECT_KIND eKind)
	{
		switch (eKind)
		{
		case ENV_OBJECT_KIND::STATIC: return kLayerEnvStatic;
		case ENV_OBJECT_KIND::INTERACT: return kLayerEnvInteract;
		case ENV_OBJECT_KIND::EFFECT: return kLayerEnvEffect;
		default: return kLayerEnvStatic;
		}
	}

	ENV_OBJECT_DESC Make_BaseDesc(const wstring& strSourceFile, const wstring& strSection, const wstring& strEntryKey)
	{
		ENV_OBJECT_DESC Desc{};
		Desc.strSourceFile = strSourceFile;
		Desc.strSection = strSection;
		Desc.strEntryKey = strEntryKey;
		Desc.eSourceType = Classify_SourceType(strSourceFile);
		Desc.iModelProtoLevel = ETOUI(TOOL_LEVEL::EDIT);
		return Desc;
	}

	void Parse_DecorEntry(const wstring& strSourceFile, const wstring& strSection, const wstring& strEntryKey, const json& jEntry, vector<ENV_OBJECT_DESC>* pOutDescs)
	{
		if (nullptr == pOutDescs)
			return;

		ENV_OBJECT_DESC Desc = Make_BaseDesc(strSourceFile, strSection, strEntryKey);
		Desc.eKind = ENV_OBJECT_KIND::STATIC;
		Desc.jRawProperties = jEntry;

		if (!Try_ReadString(jEntry, "Basic.ObjectName", &Desc.strObjectName))
			return;

		Fill_CommonFlags(jEntry, &Desc);
		Try_BuildWorldMatrixFromArray(jEntry, "WorldMtx", &Desc);
		Try_ResolveModelForObject(&Desc);
		pOutDescs->push_back(Desc);
	}

	void Parse_ToyObjEntry(const wstring& strSourceFile, const wstring& strSection, const wstring& strEntryKey, const json& jEntry, vector<ENV_OBJECT_DESC>* pOutDescs)
	{
		if (nullptr == pOutDescs)
			return;

		ENV_OBJECT_DESC Desc = Make_BaseDesc(strSourceFile, strSection, strEntryKey);
		CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(Desc);
		Desc.eKind = ENV_OBJECT_KIND::INTERACT;
		Desc.jRawProperties = jEntry;

		if (!Try_ReadString(jEntry, "Basic.ObjectName", &Desc.strObjectName))
			return;

		Fill_CommonFlags(jEntry, &Desc);
		Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.Position", &Desc.vPosition);
		Try_ReadFloat4Array(jEntry, "Basic.BasicInfo.Rotation", &Desc.vRotation);
		Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.SceneScale", &Desc.vScale);
		BaseDesc.vPosition = _float4(Desc.vPosition.x, Desc.vPosition.y, Desc.vPosition.z, 1.f);
		Try_ReadString(jEntry, "Gimmick.InteractiveDecorParts.MainComponent.MapCollType", &Desc.tCollision.strMapCollType);
		Try_ReadFloat(jEntry, "Gimmick.InteractiveDecorParts.MainComponent.MapCollRadius", &Desc.tCollision.fMapCollRadius);
		Try_ReadFloat3Array(jEntry, "Gimmick.InteractiveDecorParts.MainComponent.Size", &Desc.tCollision.vSize);
		Try_ResolveModelForObject(&Desc);
		pOutDescs->push_back(Desc);
	}

	void Parse_EffectEntry(const wstring& strSourceFile, const wstring& strSection, const wstring& strEntryKey, const json& jEntry, vector<ENV_OBJECT_DESC>* pOutDescs)
	{
		if (nullptr == pOutDescs)
			return;

		ENV_OBJECT_DESC Desc = Make_BaseDesc(strSourceFile, strSection, strEntryKey);
		CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(Desc);
		Desc.eKind = ENV_OBJECT_KIND::EFFECT;
		Desc.jRawProperties = jEntry;

		Try_ReadString(jEntry, "Basic.ObjectName", &Desc.strObjectName);
		Try_ReadString(jEntry, "Basic.BasicInfo.ObjectName", &Desc.strObjectName);
		Try_ReadString(jEntry, "ComponentName", &Desc.strComponentName);
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

		Desc.tEffect.eEffectType = Classify_EffectType(Desc.strObjectName, Desc.strComponentName);
		pOutDescs->push_back(Desc);
	}

	void Parse_SectionObject(const wstring& strSourceFile, const wstring& strSection, const json& jSection, vector<ENV_OBJECT_DESC>* pOutDescs)
	{
		if (!jSection.is_object() || nullptr == pOutDescs)
			return;

		const ENV_SOURCE_TYPE eSourceType = Classify_SourceType(strSourceFile);
		for (auto Iter = jSection.begin(); Iter != jSection.end(); ++Iter)
		{
			if (!Iter.value().is_object())
				continue;

			const wstring strEntryKey = StrToWstr(Iter.key());
			switch (eSourceType)
			{
			case ENV_SOURCE_TYPE::DECOR_DECOR:
			case ENV_SOURCE_TYPE::TOY_DECOR:
				Parse_DecorEntry(strSourceFile, strSection, strEntryKey, Iter.value(), pOutDescs);
				break;
			case ENV_SOURCE_TYPE::TOY_OBJ:
				if (0 == _wcsicmp(strSection.c_str(), L"Standard"))
					Parse_ToyObjEntry(strSourceFile, strSection, strEntryKey, Iter.value(), pOutDescs);
				break;
			case ENV_SOURCE_TYPE::DECOR_OBJ:
				Parse_EffectEntry(strSourceFile, strSection, strEntryKey, Iter.value(), pOutDescs);
				break;
			default:
				break;
			}
		}
	}
}

HRESULT CEnvObjectLoader::Ready_ObjectPrototypes(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iObjectLevel)
{
	if (nullptr == pProxy)
		return E_FAIL;

	auto EnsurePrototype = [&](const wchar_t* pProtoTag, CGameObject* pPrototype) -> HRESULT
	{
		if (pProxy->Has_Prototype(iObjectLevel, pProtoTag))
		{
			Safe_Release(pPrototype);
			return S_OK;
		}

		return pProxy->Add_Prototype(iObjectLevel, pProtoTag, pPrototype);
	};

	if (FAILED(EnsurePrototype(CEnvObject_Static::PROTOTYPE_TAG, CEnvObject_Static::Create(pDevice, pContext))))
		return E_FAIL;
	if (FAILED(EnsurePrototype(CEnvObject_Interact::PROTOTYPE_TAG, CEnvObject_Interact::Create(pDevice, pContext))))
		return E_FAIL;
	if (FAILED(EnsurePrototype(CEnvObject_Effect::PROTOTYPE_TAG, CEnvObject_Effect::Create(pDevice, pContext))))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnvObjectLoader::Build_Descriptors_FromJsonFile(const wstring& strJsonPath, vector<ENV_OBJECT_DESC>* pOutDescs)
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
		MapTool::Log_Warning(
			"EnvObject json parse failed: " + WstrToStr(strJsonPath)
			+ " reason=" + e.what());
		return E_FAIL;
	}

	wstring strSourceFile;
	Try_ReadString(jRoot, "source_file", &strSourceFile);

	const json* pData = Find_JsonValue(jRoot, "data");
	if (nullptr == pData || !pData->is_object())
		return E_FAIL;

	for (auto Iter = pData->begin(); Iter != pData->end(); ++Iter)
	{
		Parse_SectionObject(strSourceFile, StrToWstr(Iter.key()), Iter.value(), pOutDescs);
	}

	return S_OK;
}

HRESULT CEnvObjectLoader::Load_FromJsonFile(
	CGameInstance_Proxy* pProxy,
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	_uint iObjectLevel,
	const wstring& strJsonPath)
{
	if (nullptr == pProxy)
		return E_FAIL;

	if (FAILED(Ready_ObjectPrototypes(pProxy, pDevice, pContext, iObjectLevel)))
		return E_FAIL;

	vector<ENV_OBJECT_DESC> Descs;
	if (FAILED(Build_Descriptors_FromJsonFile(strJsonPath, &Descs)))
		return E_FAIL;

	_uint iCreatedCount = 0;
	_uint iSkippedMissingModel = 0;

	for (ENV_OBJECT_DESC& Desc : Descs)
	{
		if ((ENV_OBJECT_KIND::STATIC == Desc.eKind || ENV_OBJECT_KIND::INTERACT == Desc.eKind)
			&& Desc.strModelProtoTag.empty())
		{
			auto& MissingSet = Get_MissingModelLogSet();
			if (MissingSet.insert(Desc.strObjectName).second)
				MapTool::Log_Warning("EnvObject model missing: " + WstrToStr(Desc.strObjectName));

			++iSkippedMissingModel;
			continue;
		}

		if (FAILED(Ensure_ModelPrototype(pProxy, pDevice, pContext, Desc.iModelProtoLevel, Desc))
			&& !Desc.strModelProtoTag.empty())
		{
			MapTool::Log_Warning("EnvObject model prototype creation failed: " + WstrToStr(Desc.strObjectName));
			++iSkippedMissingModel;
			continue;
		}

		CGameObject* pCreatedObject = nullptr;
		const wchar_t* pProtoTag = Get_ObjectPrototypeTag(Desc.eKind);
		if (nullptr == pProtoTag)
			continue;

		if (FAILED(pProxy->Add_GameObject_Return(
			&pCreatedObject,
			iObjectLevel,
			pProtoTag,
			iObjectLevel,
			Get_ObjectLayerTag(Desc.eKind),
			Make_ObjectName(Desc),
			&Desc)))
			continue;

		++iCreatedCount;
	}

	MapTool::Log_Info(
		"EnvObject load complete: " + WstrToStr(strJsonPath)
		+ " created=" + to_string(iCreatedCount)
		+ " skipped_missing_model=" + to_string(iSkippedMissingModel));

	return S_OK;
}

NS_END
