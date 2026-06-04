#include "MapDescriptor.h"

#include "EnvObjectLoader.h"
#include "GameContent_Log.h"
#include "MapSection.h"
#include "MapStage.h"

#include "DataLoader.h"
#include "GameInstance.h"
#include "GameInstance_Proxy.h"

#include <filesystem>

namespace
{
	using namespace std::filesystem;

	constexpr _tchar kMapLayerStage[] = L"Layer_MapStage";
	constexpr _tchar kMapLayerEnvStatic[] = L"Layer_EnvStatic";
	constexpr _tchar kMapLayerEnvInteract[] = L"Layer_EnvInteract";
	constexpr _tchar kMapLayerEnvEffect[] = L"Layer_EnvEffect";
	constexpr _tchar kMapModelRoot[] = L"../../Resources/Map";
	constexpr _tchar kLegacyStageModelRoot[] = L"../../Resources/Models/Test/Stage1-0";
	constexpr _tchar kMapStageObjectTag[] = L"MapStage";

	struct MAP_DESCRIPTOR_SECTION_PRESET
	{
		const _tchar* pSectionName;
		Client::MAP_SECTION_TYPE eType;
		RENDERID eRenderID;
	};

	struct MAP_DESCRIPTOR_PRESET
	{
		const _char* pLabel;
		const _tchar* pStageName;
		const MAP_DESCRIPTOR_SECTION_PRESET* pSections;
		_uint iSectionCount;

		const _tchar* const* pEnvJsonPaths;
		_uint iEnvJsonPathCount;
	};

	constexpr MAP_DESCRIPTOR_SECTION_PRESET kStage1_0Sections[] =
	{
		{ L"GsAllBuilding_0", Client::MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND },
		{ L"GsBuilding_1", Client::MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND },
		{ L"GsBuilding_7", Client::MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND },
		{ L"GsDefault_2", Client::MAP_SECTION_TYPE::GROUND, RENDERID::NONBLEND },
		{ L"GsDefault_5", Client::MAP_SECTION_TYPE::GROUND, RENDERID::NONBLEND },
		{ L"SeRock_3", Client::MAP_SECTION_TYPE::ROCK, RENDERID::NONBLEND },
		{ L"SeRock_6", Client::MAP_SECTION_TYPE::ROCK, RENDERID::NONBLEND },
	};

	constexpr const _tchar* kStage1_0EnvJsonPaths[] =
	{
		L"../../Resources/Map/Decor_Decor_FlipZ_FullMatrix.json",
		L"../../Resources/Map/Toy_Decor.json",
		L"../../Resources/Map/Toy_Obj.json",
		L"../../Resources/Map/Decor_Obj.json",
	};

	constexpr MAP_DESCRIPTOR_PRESET kMapPresets[] =
	{
		{ "Stage1_0", L"Stage1_0_MapStage", kStage1_0Sections, _countof(kStage1_0Sections), kStage1_0EnvJsonPaths, _countof(kStage1_0EnvJsonPaths) },
	};

	const MAP_DESCRIPTOR_PRESET* Get_MapPreset(_uint iPresetIndex)
	{
		if (iPresetIndex >= _countof(kMapPresets))
			return nullptr;

		return &kMapPresets[iPresetIndex];
	}

	void Forward_GameContentLogToDebug(Client::GAMECONTENT_LOG_LEVEL, const _char* pMessage)
	{
		OutputDebugStringA("[GameContent] ");
		OutputDebugStringA(nullptr != pMessage ? pMessage : "");
		OutputDebugStringA("\n");
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
			[&](const _wstring& strExisting) { return Equals_NoCase(strExisting, strCandidate); });
		if (Iter == pOutCandidates->end())
			pOutCandidates->push_back(strCandidate);
	}

	void Build_MapSectionModelCandidates(const _wstring& strSectionName, vector<_wstring>* pOutCandidates)
	{
		if (nullptr == pOutCandidates)
			return;

		pOutCandidates->clear();
		Add_UniqueCandidate(strSectionName + L".ysh", pOutCandidates);

		if (0 == strSectionName.rfind(L"Land_", 0))
			Add_UniqueCandidate(strSectionName.substr(5) + L".ysh", pOutCandidates);
	}

	_bool Try_ResolveMapSectionModelPath(const _wstring& strSectionName, _wstring* pOutPath)
	{
		if (nullptr == pOutPath)
			return false;

		error_code ErrorCode;
		const path Root = weakly_canonical(path(kMapModelRoot), ErrorCode);
		if (ErrorCode || !exists(Root, ErrorCode))
			return false;

		vector<wstring> Candidates;
		Build_MapSectionModelCandidates(strSectionName, &Candidates);
		if (Candidates.empty())
			return false;

		ErrorCode.clear();
		for (recursive_directory_iterator Iter(Root, directory_options::skip_permission_denied, ErrorCode), End;
			Iter != End;
			Iter.increment(ErrorCode))
		{
			if (ErrorCode)
				break;

			if (!Iter->is_regular_file())
				continue;

			const wstring strFilename = Iter->path().filename().wstring();
			auto CandidateIter = find_if(
				Candidates.begin(),
				Candidates.end(),
				[&](const wstring& strCandidate) { return Equals_NoCase(strFilename, strCandidate); });
			if (CandidateIter == Candidates.end())
				continue;

			*pOutPath = Iter->path().wstring();
			return true;
		}

		return false;
	}

	_bool Try_GetLegacyMapSectionModelPath(const wstring& strSectionName, wstring* pOutPath)
	{
		if (nullptr == pOutPath)
			return false;

		const wstring strLegacyPath = wstring(kLegacyStageModelRoot) + L"/" + strSectionName + L".ysh";
		error_code ErrorCode;
		if (!exists(path(strLegacyPath), ErrorCode) || ErrorCode)
			return false;

		*pOutPath = strLegacyPath;
		return true;
	}

	wstring Make_MapSectionModelProtoTag(const wstring& strSectionName)
	{
		return L"Prototype_Component_Model_MapSection_" + strSectionName;
	}

	void Accumulate_LoadReport(const Client::ENV_OBJECT_LOAD_REPORT& EnvReport, Client::CMapDescriptor::MAP_PRESET_LOAD_REPORT* pOutReport)
	{
		if (nullptr == pOutReport)
			return;

		pOutReport->iEnvDescriptorCount += EnvReport.iDescriptorCount;
		pOutReport->iEnvCreatedCount += EnvReport.iCreatedCount;
		pOutReport->iEnvSkippedMissingModel += EnvReport.iSkippedMissingModel;
		pOutReport->iEnvSkippedCreateFailed += EnvReport.iSkippedCreateFailed;
	}
}

NS_BEGIN(Client)

IMPLEMENT_SINGLETON(CMapDescriptor)

HRESULT CMapDescriptor::Initialize()
{
	if (nullptr != m_pProxy && m_pProxy->IsConnected())
		return S_OK;

	Safe_Release(m_pProxy);
	m_pProxy = CGameInstance::GetProxy();
	return nullptr != m_pProxy ? S_OK : E_FAIL;
}

HRESULT CMapDescriptor::Bulid_MapStageDesc(_uint iPresetIndex, _uint iSectionProtoLevel, _uint iModelLevel, MAP_STAGE_DESC* pOutStageDesc)
{
	if (nullptr == pOutStageDesc)
		return E_FAIL;

	const MAP_DESCRIPTOR_PRESET* pPreset = Get_MapPreset(iPresetIndex);
	if (nullptr == pPreset)
		return E_FAIL;

	pOutStageDesc->strStageName = pPreset->pStageName;
	pOutStageDesc->iSectionProtoLevel = iSectionProtoLevel;
	pOutStageDesc->SectionDescs.clear();
	pOutStageDesc->SectionDescs.reserve(pPreset->iSectionCount);

	for (_uint i = 0; i < pPreset->iSectionCount; ++i)
	{
		const MAP_DESCRIPTOR_SECTION_PRESET& SectionPreset = pPreset->pSections[i];
		const wstring strSectionName = SectionPreset.pSectionName;

		wstring strModelPath;
		const _bool bHasLegacyPath = Try_GetLegacyMapSectionModelPath(strSectionName, &strModelPath);
		if (!bHasLegacyPath && !Try_ResolveMapSectionModelPath(strSectionName, &strModelPath))
		{
			Log_GameContentWarning("Map preset section model missing: " + WstrToStr(strSectionName));
			return E_FAIL;
		}

		MAP_SECTION_DESC SectionDesc{};
		SectionDesc.strSectionName = strSectionName;
		SectionDesc.strModelProtoTag = bHasLegacyPath
			? L"Prototype_Component_Model_" + strSectionName
			: Make_MapSectionModelProtoTag(strSectionName);
		SectionDesc.strModelPath = strModelPath;
		SectionDesc.iModelProtoLevel = iModelLevel;
		SectionDesc.eSectionType = SectionPreset.eType;
		SectionDesc.eRenderID = SectionPreset.eRenderID;
		SectionDesc.bCastShadow = false;
		SectionDesc.bEnableCulling = true;
		SectionDesc.bRenderable = true;

		pOutStageDesc->SectionDescs.push_back(SectionDesc);
	}

	return S_OK;
}

HRESULT CMapDescriptor::Load_MapStage(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	_uint iPresetIndex,
	_uint iPlaceLevel,
	_uint iModelLevel,
	CMapDescriptor::MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback,
	void* pCallbackContext,
	CMapStage** ppOutStage,
	_wstring* pOutStageName)
{
	if (FAILED(Ensure_Initialized()))
		return E_FAIL;

	if (FAILED(Ready_MapStagePrototype(pDevice, pContext, iPlaceLevel)))
		return E_FAIL;

	MAP_STAGE_DESC StageDesc{};
	if (FAILED(Bulid_MapStageDesc(iPresetIndex, iPlaceLevel, iModelLevel, &StageDesc)))
		return E_FAIL;

	CGameObject* pStageObject = nullptr;
	if (FAILED(m_pProxy->Add_GameObject_Return(
		&pStageObject,
		iPlaceLevel,
		CMapStage::PROTOTYPE_TAG,
		iPlaceLevel,
		kMapLayerStage,
		kMapStageObjectTag,
		&StageDesc)))
	{
		return E_FAIL;
	}

	CMapStage* pStage = dynamic_cast<CMapStage*>(pStageObject);
	if (nullptr == pStage)
	{
		if (nullptr != pStageObject)
			m_pProxy->Destroy_GameObject(pStageObject);
		return E_FAIL;
	}

	if (nullptr != ppOutStage)
		*ppOutStage = pStage;

	if (nullptr != pOutStageName)
		*pOutStageName = StageDesc.strStageName;

	if (nullptr != pCreatedCallback)
		pCreatedCallback(pCallbackContext, pStage, CMapStage::PROTOTYPE_TAG, kMapLayerStage, kMapStageObjectTag);

	return S_OK;
}

HRESULT CMapDescriptor::Load_EnvObject_FromJson(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	_uint iPresetIndex,
	_uint iPlaceLevel,
	CMapDescriptor::MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback,
	void* pCallbackContext,
	CMapDescriptor::MAP_PRESET_LOAD_REPORT* pOutReport)
{
	if (FAILED(Ensure_Initialized()))
		return E_FAIL;

	const MAP_DESCRIPTOR_PRESET* pPreset = Get_MapPreset(iPresetIndex);
	if (nullptr == pPreset)
		return E_FAIL;

	HRESULT hResult = S_OK;
	Set_GameContentLogSink(&Forward_GameContentLogToDebug);

	for (_uint i = 0; i < pPreset->iEnvJsonPathCount; ++i)
	{
		const wchar_t* pJsonPath = pPreset->pEnvJsonPaths[i];

		string strDummy;
		if (FAILED(CDataLoader::Read_Json(pJsonPath, &strDummy)))
		{
			if (nullptr != pOutReport)
				++pOutReport->iEnvJsonFailedCount;
			hResult = S_FALSE;
			continue;
		}

		ENV_OBJECT_LOAD_REPORT EnvReport{};
		if (FAILED(CEnvObjectLoader::Load_FromJsonFile(
			m_pProxy,
			pDevice,
			pContext,
			iPlaceLevel,
			pJsonPath,
			pCreatedCallback,
			pCallbackContext,
			&EnvReport)))
		{
			if (nullptr != pOutReport)
				++pOutReport->iEnvJsonFailedCount;
			hResult = S_FALSE;
			continue;
		}

		if (nullptr != pOutReport)
			++pOutReport->iEnvJsonLoadedCount;

		Accumulate_LoadReport(EnvReport, pOutReport);
		if (0 != EnvReport.iSkippedMissingModel || 0 != EnvReport.iSkippedCreateFailed)
			hResult = S_FALSE;
	}

	return hResult;
}

HRESULT CMapDescriptor::Load_MapPreset(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	_uint iPresetIndex,
	_uint iPlaceLevel,
	_uint iModelLevel,
	CMapDescriptor::MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback,
	void* pCallbackContext,
	CMapDescriptor::MAP_PRESET_LOAD_REPORT* pOutReport,
	CMapStage** ppOutStage)
{
	if (nullptr != pOutReport)
		*pOutReport = {};

	_wstring strStageName;
	HRESULT hResult = Load_MapStage(
		pDevice,
		pContext,
		iPresetIndex,
		iPlaceLevel,
		iModelLevel,
		pCreatedCallback,
		pCallbackContext,
		ppOutStage,
		&strStageName);
	if (FAILED(hResult))
		return hResult;

	if (nullptr != pOutReport)
	{
		pOutReport->bStageLoaded = true;
		pOutReport->strStageName = strStageName;
	}

	hResult = Load_EnvObject_FromJson(
		pDevice,
		pContext,
		iPresetIndex,
		iPlaceLevel,
		pCreatedCallback,
		pCallbackContext,
		pOutReport);
	return hResult;
}

_bool CMapDescriptor::Is_MapLayer(const wstring& strLayerTag) const
{
	return strLayerTag == kMapLayerStage
		|| strLayerTag == kMapLayerEnvStatic
		|| strLayerTag == kMapLayerEnvInteract
		|| strLayerTag == kMapLayerEnvEffect;
}

_uint CMapDescriptor::Get_MapPresetCount() const
{
	return static_cast<_uint>(_countof(kMapPresets));
}

const char* CMapDescriptor::Get_MapPresetLabel(_uint iPresetIndex) const
{
	const MAP_DESCRIPTOR_PRESET* pPreset = Get_MapPreset(iPresetIndex);
	return nullptr != pPreset ? pPreset->pLabel : "Unknown";
}

HRESULT CMapDescriptor::Ready_MapStagePrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iPlaceLevel)
{
	if (FAILED(Ensure_Initialized()))
		return E_FAIL;

	auto EnsurePrototype = [&](const wchar_t* pPrototypeTag, CGameObject* pPrototype) -> HRESULT
		{
			if (nullptr == pPrototype)
				return E_FAIL;

			if (m_pProxy->Has_Prototype(iPlaceLevel, pPrototypeTag))
			{
				Safe_Release(pPrototype);
				return S_OK;
			}

			if (FAILED(m_pProxy->Add_Prototype(iPlaceLevel, pPrototypeTag, pPrototype)))
			{
				Safe_Release(pPrototype);
				return E_FAIL;
			}

			return S_OK;
		};

	if (FAILED(EnsurePrototype(CMapSection::PROTOTYPE_TAG, CMapSection::Create(pDevice, pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CMapStage::PROTOTYPE_TAG, CMapStage::Create(pDevice, pContext))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapDescriptor::Ensure_Initialized()
{
	return Initialize();
}

void CMapDescriptor::Free()
{
	Safe_Release(m_pProxy);
	__super::Free();
}

NS_END
