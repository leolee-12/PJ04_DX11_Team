#include "Map_EditHelper.h"
#include "EnvObject_Effect.h"
#include "EnvObject_Interact.h"
#include "EnvObject_Static.h"
#include "GameContent_Log.h"
#include "Map_ModelResolver.h"
#include "Map_Parser.h"
#include "Map_ProtoRegister.h"
#include "Map_Spawner.h"
#include "MapStage.h"

#include "GameInstance.h"

#include <exception>
#include <unordered_set>

NS_BEGIN(Client)

namespace
{
    using namespace std::filesystem;

    constexpr _tchar kMapLayerStage[] = L"Layer_MapStage";
    constexpr _tchar kMapLayerEnvStatic[] = L"Layer_EnvStatic";
    constexpr _tchar kMapLayerEnvInteract[] = L"Layer_EnvInteract";
    constexpr _tchar kMapLayerEnvEffect[] = L"Layer_EnvEffect";
    constexpr _tchar kMapModelRoot[] = L"../../Resources/Map";
    constexpr _tchar kMapStageObjectTag[] = L"MapStage";

    struct MAP_DESCRIPTOR_SECTION_PRESET
    {
        const _tchar* pSectionName;
        MAP_SECTION_TYPE eType;
        RENDERID eRenderID;
    };

    struct MAP_DESCRIPTOR_PRESET
    {
        const _char* pLabel;
        const _tchar* pStageName;
        const _tchar* pStageFolderName;
        const MAP_DESCRIPTOR_SECTION_PRESET* pSections;
        _uint iSectionCount;

        const _tchar* const* pEnvJsonPaths;
        _uint iEnvJsonPathCount;
    };

    constexpr MAP_DESCRIPTOR_SECTION_PRESET kStage1_1Sections[] =
    {
            { L"GsAllBuilding_0", MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND },
            { L"GsBuilding_1", MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND },
            { L"GsBuilding_7", MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND },
            { L"GsDefault_2", MAP_SECTION_TYPE::GROUND, RENDERID::NONBLEND },
            { L"GsDefault_5", MAP_SECTION_TYPE::GROUND, RENDERID::NONBLEND },
            { L"SeRock_3", MAP_SECTION_TYPE::ROCK, RENDERID::NONBLEND },
            { L"SeRock_6", MAP_SECTION_TYPE::ROCK, RENDERID::NONBLEND },
    };

    constexpr const _tchar* kStage1_1EnvJsonPaths[] =
    {
            L"../../Resources/Map/Stage1-1/Decor_Decor_FlipZ.json",
            L"../../Resources/Map/Stage1-1/Toy_Decor_FlipZ.json",
            L"../../Resources/Map/Stage1-1/Toy_Obj_FlipZ.json",
            L"../../Resources/Map/Stage1-1/Decor_Obj_FlipZ.json",
    };

    constexpr MAP_DESCRIPTOR_PRESET kMapPresets[] =
    {
            {
                    "Stage1-1",
                    L"Stage1-1_MapStage",
                    L"Stage1-1",
                    kStage1_1Sections,
                    _countof(kStage1_1Sections),
                    kStage1_1EnvJsonPaths,
                    _countof(kStage1_1EnvJsonPaths)
            },
    };

    const MAP_DESCRIPTOR_PRESET* Get_MapPreset(_uint iPresetIndex)
    {
        if (iPresetIndex >= _countof(kMapPresets))
            return nullptr;

        return &kMapPresets[iPresetIndex];
    }

    void Forward_GameContentLogToDebug(GAMECONTENT_LOG_LEVEL, const _char* pMessage)
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
            [&](const _wstring& strExisting)
            {
                return Equals_NoCase(strExisting, strCandidate);
            });

        if (Iter == pOutCandidates->end())
            pOutCandidates->push_back(strCandidate);
    }

    void Build_MapSectionModelCandidates(const _wstring& strSectionName, vector<_wstring>*
        pOutCandidates)
    {
        if (nullptr == pOutCandidates)
            return;

        pOutCandidates->clear();
        Add_UniqueCandidate(strSectionName + L".ysh", pOutCandidates);

        if (0 == strSectionName.rfind(L"Land_", 0))
            Add_UniqueCandidate(strSectionName.substr(5) + L".ysh", pOutCandidates);
    }

    _bool Try_ResolveMapSectionModelPath(
        const _wstring& strStageFolderName,
        const _wstring& strSectionName,
        _wstring* pOutPath)
    {
        if (nullptr == pOutPath || strStageFolderName.empty())
            return false;

        vector<_wstring> Candidates;
        Build_MapSectionModelCandidates(strSectionName, &Candidates);
        if (Candidates.empty())
            return false;

        const path SectionRoot = path(kMapModelRoot) / strStageFolderName / L"Section";

        error_code ErrorCode;
        if (!exists(SectionRoot, ErrorCode) || ErrorCode)
            return false;

        for (const _wstring& strCandidate : Candidates)
        {
            const path CandidatePath = SectionRoot / strCandidate;

            ErrorCode.clear();
            if (exists(CandidatePath, ErrorCode) && !ErrorCode)
            {
                *pOutPath = CandidatePath.wstring();
                return true;
            }
        }

        return false;
    }

    _wstring Make_MapSectionModelProtoTag(const _wstring& strStageFolderName, const _wstring&
        strSectionName)
    {
        return L"Prototype_Component_Model_MapSection_" + strStageFolderName + L"_" + strSectionName;
    }

    const _tchar* Get_EnvLayerTag(ENV_OBJECT_KIND eKind)
    {
        switch (eKind)
        {
        case ENV_OBJECT_KIND::STATIC: return kMapLayerEnvStatic;
        case ENV_OBJECT_KIND::INTERACT: return kMapLayerEnvInteract;
        case ENV_OBJECT_KIND::EFFECT: return kMapLayerEnvEffect;
        default: return nullptr;
        }
    }

    const _tchar* Get_EnvObjectProtoTag(ENV_OBJECT_KIND eKind)
    {
        switch (eKind)
        {
        case ENV_OBJECT_KIND::STATIC: return CEnvObject_Static::PROTOTYPE_TAG;
        case ENV_OBJECT_KIND::INTERACT: return CEnvObject_Interact::PROTOTYPE_TAG;
        case ENV_OBJECT_KIND::EFFECT: return CEnvObject_Effect::PROTOTYPE_TAG;
        default: return nullptr;
        }
    }

    _wstring Make_EnvObjectName(const ENV_OBJECT_DESC& Desc)
    {
        wstring strName = L"Env_" + Desc.strObjectName;
        if (0 != Desc.iUid)
            strName += L"_" + to_wstring(Desc.iUid);
        else if (!Desc.strEntryKey.empty())
            strName += L"_" + Desc.strEntryKey;
        return strName;
    }

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

    void Accumulate_LoadReport(const MAP_LOAD_REPORT& Report, CMap_EditHelper::MAP_PRESET_LOAD_REPORT*
        pOutReport)
    {
        if (nullptr == pOutReport)
            return;

        pOutReport->bStageLoaded = pOutReport->bStageLoaded || Report.bStageLoaded;

        if (pOutReport->strStageName.empty() && !Report.strStageName.empty())
            pOutReport->strStageName = Report.strStageName;

        pOutReport->iEnvJsonLoadedCount += Report.iEnvJsonLoadedCount;
        pOutReport->iEnvDescriptorCount += Report.iEnvDescriptorCount;
        pOutReport->iEnvCreatedCount += Report.iEnvCreatedCount;
        pOutReport->iEnvSkippedMissingModel += Report.iEnvSkippedMissingModel;
        pOutReport->iEnvSkippedCreateFailed += Report.iEnvSkippedCreateFailed;
    }

    unordered_set<_wstring>& Get_MissingModelLogSet()
    {
        static unordered_set<_wstring> s_Set;
        return s_Set;
    }

    HRESULT Ready_EnvObjectPrototypes(
        CGameInstance_Proxy* pProxy,
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iObjectLevel)
    {
        if (nullptr == pProxy || nullptr == pDevice || nullptr == pContext)
            return E_FAIL;

        auto EnsurePrototype = [&](const wchar_t* pProtoTag, CGameObject* pPrototype) -> HRESULT
            {
                if (nullptr == pPrototype)
                    return E_FAIL;

                if (pProxy->Has_Prototype(iObjectLevel, pProtoTag))
                {
                    Safe_Release(pPrototype);
                    return S_OK;
                }

                if (FAILED(pProxy->Add_Prototype(iObjectLevel, pProtoTag, pPrototype)))
                {
                    Safe_Release(pPrototype);
                    return E_FAIL;
                }

                return S_OK;
            };

        if (FAILED(EnsurePrototype(CEnvObject_Static::PROTOTYPE_TAG, CEnvObject_Static::Create(pDevice,
            pContext))))
            return E_FAIL;
        if (FAILED(EnsurePrototype(CEnvObject_Interact::PROTOTYPE_TAG, CEnvObject_Interact::Create(pDevice,
            pContext))))
            return E_FAIL;
        if (FAILED(EnsurePrototype(CEnvObject_Effect::PROTOTYPE_TAG, CEnvObject_Effect::Create(pDevice,
            pContext))))
            return E_FAIL;

        return S_OK;
    }

    HRESULT Ensure_EnvModelPrototype(
        CGameInstance_Proxy* pProxy,
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        _uint iModelLevel,
        const ENV_OBJECT_DESC& Desc)
    {
        if (nullptr == pProxy || nullptr == pDevice || nullptr == pContext)
            return E_FAIL;

        if (Desc.strModelProtoTag.empty() || Desc.strModelPath.empty())
            return S_FALSE;

        if (pProxy->Has_Prototype(iModelLevel, Desc.strModelProtoTag))
            return S_OK;

        const string strModelPath = WstrToStr(Desc.strModelPath);
        CModel* pModelPrototype = nullptr;

        try
        {
            pModelPrototype = CModel::Create_WithTextureHub(
                pDevice,
                pContext,
                MODEL::NONANIM,
                strModelPath.c_str());
        }
        catch (const std::exception& e)
        {
            Log_GameContentWarning(
                "Map_EditHelper env model creation exception: object="
                + WstrToStr(Describe_EnvObject(Desc))
                + " path=" + strModelPath
                + " reason=" + e.what());
            return E_FAIL;
        }

        if (nullptr == pModelPrototype)
            return E_FAIL;

        if (FAILED(pProxy->Add_Prototype(iModelLevel, Desc.strModelProtoTag.c_str(), pModelPrototype)))
        {
            Safe_Release(pModelPrototype);
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT Build_EnvDescriptors(
        const _wstring& strJsonPath,
        const _wstring& strStageFolderName,
        vector<ENV_OBJECT_DESC>* pOutDescs)
    {
        if (nullptr == pOutDescs)
            return E_FAIL;

        pOutDescs->clear();

        CMap_ModelResolver* pResolver = CMap_ModelResolver::Create(kMapModelRoot);
        if (nullptr == pResolver)
            return E_FAIL;

        HRESULT hr = pResolver->Build_EnvModelCache(strStageFolderName);
        if (FAILED(hr))
        {
            Safe_Release(pResolver);
            return hr;
        }

        hr = CMap_Parser::Parse_EnvJson(strJsonPath, pOutDescs);
        if (FAILED(hr))
        {
            Safe_Release(pResolver);
            return hr;
        }

        for (ENV_OBJECT_DESC& Desc : *pOutDescs)
            pResolver->Resolve_EnvObject(&Desc);

        Safe_Release(pResolver);
        return S_OK;
    }

    HRESULT Build_PresetStageDesc(_uint iPresetIndex, MAP_STAGE_DESC* pOutStageDesc)
    {
        if (nullptr == pOutStageDesc)
            return E_FAIL;

        const MAP_DESCRIPTOR_PRESET* pPreset = Get_MapPreset(iPresetIndex);
        if (nullptr == pPreset)
            return E_FAIL;

        pOutStageDesc->strStageName = pPreset->pStageName;
        pOutStageDesc->iSectionProtoLevel = 0;
        pOutStageDesc->SectionDescs.clear();
        pOutStageDesc->SectionDescs.reserve(pPreset->iSectionCount);

        for (_uint i = 0; i < pPreset->iSectionCount; ++i)
        {
            const MAP_DESCRIPTOR_SECTION_PRESET& SectionPreset = pPreset->pSections[i];
            const _wstring strSectionName = SectionPreset.pSectionName;

            _wstring strModelPath;
            if (!Try_ResolveMapSectionModelPath(pPreset->pStageFolderName, strSectionName, &strModelPath))
            {
                Log_GameContentWarning(
                    "Map preset section model missing: stage="
                    + WstrToStr(pPreset->pStageFolderName)
                    + " section="
                    + WstrToStr(strSectionName));
                return E_FAIL;
            }

            MAP_SECTION_DESC SectionDesc{};
            SectionDesc.strSectionName = strSectionName;
            SectionDesc.strModelProtoTag = Make_MapSectionModelProtoTag(pPreset->pStageFolderName,
                strSectionName);
            SectionDesc.strModelPath = strModelPath;
            SectionDesc.iModelProtoLevel = 0;
            SectionDesc.eSectionType = SectionPreset.eType;
            SectionDesc.eRenderID = SectionPreset.eRenderID;
            SectionDesc.bCastShadow = true;
            SectionDesc.bEnableCulling = true;
            SectionDesc.bRenderable = true;

            pOutStageDesc->SectionDescs.push_back(SectionDesc);
        }

        return S_OK;
    }

    HRESULT Load_PresetEnvJsons(
        CGameInstance_Proxy* pProxy,
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const MAP_DESCRIPTOR_PRESET& Preset,
        _uint iPlaceLevel,
        CMap_EditHelper::MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback,
        void* pCallbackContext,
        CMap_EditHelper::MAP_PRESET_LOAD_REPORT* pOutReport)
    {
        CMap_EditHelper::MAP_PRESET_LOAD_REPORT LocalReport{};
        CMap_EditHelper::MAP_PRESET_LOAD_REPORT* pReport = nullptr != pOutReport ? pOutReport :
            &LocalReport;

        _bool bAnySucceeded = false;

        for (_uint i = 0; i < Preset.iEnvJsonPathCount; ++i)
        {
            const _wstring strJsonPath = Preset.pEnvJsonPaths[i];

            MAP_LOAD_REPORT StepReport{};
            const HRESULT hr = CMap_EditHelper::Import_EnvJson_Immediate(
                pProxy,
                pDevice,
                pContext,
                iPlaceLevel,
                strJsonPath,
                Preset.pStageFolderName,
                pCreatedCallback,
                pCallbackContext,
                &StepReport);

            if (FAILED(hr))
            {
                ++pReport->iEnvJsonFailedCount;
                Log_GameContentWarning("Map_EditHelper env json import failed: " + WstrToStr(strJsonPath));
                continue;
            }

            bAnySucceeded = true;
            Accumulate_LoadReport(StepReport, pReport);
        }

        if (0 == Preset.iEnvJsonPathCount)
            return S_OK;

        return bAnySucceeded ? S_OK : E_FAIL;
    }
}

_bool CMap_EditHelper::Is_MapLayer(const wstring& strLayerTag)
{
    return strLayerTag == kMapLayerStage
        || strLayerTag == kMapLayerEnvStatic
        || strLayerTag == kMapLayerEnvInteract
        || strLayerTag == kMapLayerEnvEffect;
}

_uint CMap_EditHelper::Get_MapPresetCount()
{
    return static_cast<_uint>(_countof(kMapPresets));
}

const _char* CMap_EditHelper::Get_MapPresetLabel(_uint iPresetIndex)
{
    const MAP_DESCRIPTOR_PRESET* pPreset = Get_MapPreset(iPresetIndex);
    return nullptr != pPreset ? pPreset->pLabel : "Unknown";
}

HRESULT CMap_EditHelper::Load_MapStage(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    _uint iPresetIndex,
    _uint iPlaceLevel,
    _uint iModelLevel,
    MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback,
    void* pCallbackContext,
    CMapStage** ppOutStage,
    _wstring* pOutStageName)
{
    if (nullptr == pDevice || nullptr == pContext)
        return E_FAIL;

    if (nullptr != ppOutStage)
        *ppOutStage = nullptr;

    if (nullptr != pOutStageName)
        pOutStageName->clear();

    Set_GameContentLogSink(&Forward_GameContentLogToDebug);

    MAP_PACKAGE Package{};
    if (FAILED(Build_PresetStageDesc(iPresetIndex, &Package.StageDesc)))
        return E_FAIL;

    MAP_RUNTIME_LEVELS Levels{};
    Levels.iObjectLevel = iPlaceLevel;
    Levels.iStageModelLevel = iModelLevel;
    Levels.iEnvModelLevel = iPlaceLevel;

    CMap_ProtoRegister* pRegister = CMap_ProtoRegister::Create(pDevice, pContext);
    if (nullptr == pRegister)
        return E_FAIL;

    HRESULT hr = pRegister->Ready_Prototypes(Levels, Package);
    Safe_Release(pRegister);
    if (FAILED(hr))
        return hr;

    MAP_SPAWN_REQUEST Request{};
    Request.Levels = Levels;
    Request.Targets.Stage.iPlaceLevel = iPlaceLevel;
    Request.Targets.Stage.pLayerTag = kMapLayerStage;
    Request.Targets.pStageObjectTag = kMapStageObjectTag;
    Request.Options.bSpawnStage = true;
    Request.Options.bSpawnEnv = false;
    Request.pCreatedCallback = pCreatedCallback;
    Request.pCallbackContext = pCallbackContext;
    Request.ppOutStage = ppOutStage;

    CMap_Spawner* pSpawner = CMap_Spawner::Create();
    if (nullptr == pSpawner)
        return E_FAIL;

    hr = pSpawner->Spawn(Package, Request, nullptr);
    Safe_Release(pSpawner);
    if (FAILED(hr))
        return hr;

    if (nullptr != pOutStageName)
        *pOutStageName = Package.StageDesc.strStageName;

    return S_OK;
}

HRESULT CMap_EditHelper::Load_EnvObject_FromJson(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    _uint iPresetIndex,
    _uint iPlaceLevel,
    MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback,
    void* pCallbackContext,
    MAP_PRESET_LOAD_REPORT* pOutReport)
{
    if (nullptr == pDevice || nullptr == pContext)
        return E_FAIL;

    MAP_PRESET_LOAD_REPORT LocalReport{};
    MAP_PRESET_LOAD_REPORT* pReport = nullptr != pOutReport ? pOutReport : &LocalReport;
    *pReport = {};

    Set_GameContentLogSink(&Forward_GameContentLogToDebug);

    CGameInstance_Proxy* pProxy = CGameInstance::GetProxy();
    if (nullptr == pProxy)
        return E_FAIL;

    const MAP_DESCRIPTOR_PRESET* pPreset = Get_MapPreset(iPresetIndex);
    if (nullptr == pPreset)
    {
        Safe_Release(pProxy);
        return E_FAIL;
    }

    const HRESULT hr = Load_PresetEnvJsons(
        pProxy,
        pDevice,
        pContext,
        *pPreset,
        iPlaceLevel,
        pCreatedCallback,
        pCallbackContext,
        pReport);

    Safe_Release(pProxy);
    return hr;
}

HRESULT CMap_EditHelper::Load_MapPreset(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    _uint iPresetIndex,
    _uint iPlaceLevel,
    _uint iModelLevel,
    MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback,
    void* pCallbackContext,
    MAP_PRESET_LOAD_REPORT* pOutReport,
    CMapStage** ppOutStage)
{
    if (nullptr == pDevice || nullptr == pContext)
        return E_FAIL;

    MAP_PRESET_LOAD_REPORT LocalReport{};
    MAP_PRESET_LOAD_REPORT* pReport = nullptr != pOutReport ? pOutReport : &LocalReport;
    *pReport = {};

    if (nullptr != ppOutStage)
        *ppOutStage = nullptr;

    Set_GameContentLogSink(&Forward_GameContentLogToDebug);

    const MAP_DESCRIPTOR_PRESET* pPreset = Get_MapPreset(iPresetIndex);
    if (nullptr == pPreset)
        return E_FAIL;

    CMapStage* pLoadedStage = nullptr;
    _wstring strStageName;
    if (FAILED(Load_MapStage(
        pDevice,
        pContext,
        iPresetIndex,
        iPlaceLevel,
        iModelLevel,
        pCreatedCallback,
        pCallbackContext,
        &pLoadedStage,
        &strStageName)))
    {
        return E_FAIL;
    }

    pReport->bStageLoaded = true;
    pReport->strStageName = strStageName;

    if (nullptr != ppOutStage)
        *ppOutStage = pLoadedStage;

    CGameInstance_Proxy* pProxy = CGameInstance::GetProxy();
    if (nullptr == pProxy)
        return S_OK;

    MAP_PRESET_LOAD_REPORT EnvReport{};
    const HRESULT hrEnv = Load_PresetEnvJsons(
        pProxy,
        pDevice,
        pContext,
        *pPreset,
        iPlaceLevel,
        pCreatedCallback,
        pCallbackContext,
        &EnvReport);

    Safe_Release(pProxy);

    pReport->iEnvJsonLoadedCount += EnvReport.iEnvJsonLoadedCount;
    pReport->iEnvJsonFailedCount += EnvReport.iEnvJsonFailedCount;
    pReport->iEnvDescriptorCount += EnvReport.iEnvDescriptorCount;
    pReport->iEnvCreatedCount += EnvReport.iEnvCreatedCount;
    pReport->iEnvSkippedMissingModel += EnvReport.iEnvSkippedMissingModel;
    pReport->iEnvSkippedCreateFailed += EnvReport.iEnvSkippedCreateFailed;

    if (FAILED(hrEnv))
    {
        Log_GameContentWarning(
            "Map_EditHelper preset env import completed with failures: preset="
            + string(Get_MapPresetLabel(iPresetIndex)));
    }

    return S_OK;
}

HRESULT CMap_EditHelper::Import_EnvJson_Immediate(
    CGameInstance_Proxy* pProxy,
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    _uint iObjectLevel,
    const _wstring& strJsonPath,
    const _wstring& strStageFolderName,
    MAP_OBJECT_CREATED_CALLBACK pCreatedCallback,
    void* pCallbackContext,
    MAP_LOAD_REPORT* pOutReport)
{
    if (nullptr == pProxy || nullptr == pDevice || nullptr == pContext)
        return E_FAIL;

    if (strJsonPath.empty())
        return E_FAIL;

    if (nullptr != pOutReport)
        *pOutReport = {};

    if (FAILED(Ready_EnvObjectPrototypes(pProxy, pDevice, pContext, iObjectLevel)))
        return E_FAIL;

    vector<ENV_OBJECT_DESC> Descs;
    if (FAILED(Build_EnvDescriptors(strJsonPath, strStageFolderName, &Descs)))
        return E_FAIL;

    MAP_LOAD_REPORT Report{};
    Report.iEnvJsonLoadedCount = 1;
    Report.iEnvDescriptorCount = static_cast<_uint>(Descs.size());

    for (ENV_OBJECT_DESC& Desc : Descs)
    {
        Desc.iModelProtoLevel = iObjectLevel;

        if (Needs_Model(Desc) && (Desc.strModelPath.empty() || Desc.strModelProtoTag.empty()))
        {
            const _wstring strObjectLabel = Describe_EnvObject(Desc);
            if (Get_MissingModelLogSet().insert(strObjectLabel).second)
            {
                Log_GameContentWarning(
                    "Map_EditHelper skipped env without model: object="
                    + WstrToStr(strObjectLabel));
            }

            ++Report.iEnvSkippedMissingModel;
            continue;
        }

        const HRESULT hrModel = Ensure_EnvModelPrototype(
            pProxy,
            pDevice,
            pContext,
            Desc.iModelProtoLevel,
            Desc);

        if (FAILED(hrModel) && !Desc.strModelProtoTag.empty())
        {
            Log_GameContentWarning(
                "Map_EditHelper env model prototype creation failed: object="
                + WstrToStr(Describe_EnvObject(Desc))
                + " path="
                + WstrToStr(Desc.strModelPath));

            ++Report.iEnvSkippedMissingModel;
            continue;
        }

        const _tchar* pProtoTag = Get_EnvObjectProtoTag(Desc.eKind);
        const _tchar* pLayerTag = Get_EnvLayerTag(Desc.eKind);
        if (nullptr == pProtoTag || nullptr == pLayerTag)
        {
            ++Report.iEnvSkippedCreateFailed;
            continue;
        }

        CGameObject* pCreatedObject = nullptr;
        const _wstring strObjectTag = Make_EnvObjectName(Desc);

        if (FAILED(pProxy->Add_GameObject_Return(
            &pCreatedObject,
            iObjectLevel,
            pProtoTag,
            iObjectLevel,
            pLayerTag,
            strObjectTag,
            &Desc)))
        {
            ++Report.iEnvSkippedCreateFailed;
            continue;
        }

        ++Report.iEnvCreatedCount;

        if (nullptr != pCreatedCallback)
        {
            pCreatedCallback(
                pCallbackContext,
                pCreatedObject,
                pProtoTag,
                pLayerTag,
                strObjectTag);
        }
    }

    if (nullptr != pOutReport)
        *pOutReport = Report;

    Log_GameContentInfo(
        "Map_EditHelper env import complete: " + WstrToStr(strJsonPath)
        + " created=" + to_string(Report.iEnvCreatedCount)
        + " skipped_missing_model=" + to_string(Report.iEnvSkippedMissingModel)
        + " skipped_create_failed=" + to_string(Report.iEnvSkippedCreateFailed));

    return S_OK;
}

json CMap_EditHelper::Serialize_MapStageOverride(const CMapStage*)
{
    return json{};
}

HRESULT CMap_EditHelper::Apply_MapStageOverride(CMapStage*, const json&)
{
    return E_NOTIMPL;
}

NS_END