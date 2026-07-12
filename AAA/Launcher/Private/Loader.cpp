#include "Loader.h"
#include "GameContent_const.h"
#include "GameInstance.h"
#include "GameObject_Factory.h"
#include "DataLoader.h"
#include "Map_Loader.h"
#include "Loader_Prototype.h"
#include "Launcher_LevelProfiles.h"
#include "Map_Loader.h"
#include <set>

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
    , m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

unsigned int APIENTRY ThreadMain(void* pArg)
{
    CLoader*        pLoader = static_cast<CLoader*>(pArg);

    if (FAILED(pLoader->Loading()))
        return -1;

    return 0;
}

unsigned int APIENTRY CollectorMain(void* pArg)
{
    CLoader* pLoader = static_cast<CLoader*>(pArg);
    if (FAILED(CoInitializeEx(nullptr, 0)))
        return -1;
    const HRESULT hr = pLoader->Collect_And_StartWorkers();
    CoUninitialize();
    return FAILED(hr) ? -1 : 0;
}



HRESULT CLoader::Initialize(LEVEL eNextLevelID, _bool Initialized)
{
    m_eNextLevelID = eNextLevelID;
    m_bNeedStatic = !Initialized;

    m_hCollector = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, CollectorMain, this, 0, nullptr));
    if (0 == m_hCollector)
        return E_FAIL;

    SetThreadPriority(m_hCollector, THREAD_PRIORITY_BELOW_NORMAL);
    return S_OK;
}

HRESULT CLoader::Loading()
{
    if (FAILED(CoInitializeEx(nullptr, 0)))
        return E_FAIL;

    while (true)
    {
        function<HRESULT()> func;
        if (m_WorkQueue.try_pop(func))
        {
            if (FAILED(func()))
                int debugpoint = 10;
            ++m_iFinishedWorkCount;
        }
        else
        {
            break;
		}
    }

    CoUninitialize();

    return S_OK;
}

#ifdef _DEBUG

void CLoader::Show()
{
    //SetWindowText(g_hWnd, m_szLoadingText);
}

#endif

void CLoader::Add_Work(function<HRESULT()>&& func)
{
    m_WorkQueue.push(move(func));
    ++m_iTotalWorkCount;
}

HRESULT CLoader::Read_Manifest(const _tchar* path, const LEVEL eLevel)
{
    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(path, &Manifest)))
        return E_FAIL;

    vector<function<HRESULT()>> MapJobs;
    if (SUCCEEDED(CMap_Loader::Collect_PreloadJobs(m_pDevice, m_pContext,
        Manifest.strMapManifest, Manifest.strObjectsFile, ETOUI(eLevel), &MapJobs)))
    {
        for (auto& Job : MapJobs)
            Add_Work(move(Job));
    }
    else
    {
        Add_Work([this, Manifest, eLevel]() -> HRESULT
            {
                return CMap_Loader::Preload_Map(m_pDevice, m_pContext,
                    Manifest.strMapManifest, Manifest.strObjectsFile, ETOUI(eLevel));
            });
    }

    string strContent;
    CDataLoader::Read_Json(Manifest.strObjectsFile.c_str(), &strContent);
    json jLevel = json::parse(strContent);

    set<wstring> visited;
    for (auto& jObj : jLevel["Objects"])
    {
        wstring wProto = StrToWstr(jObj["Prototype_Tag"].get<string>());
        if (!visited.insert(wProto).second) continue;

        if (m_pGameInstance_Proxy->Has_Prototype(ETOUI(eLevel), wProto)) continue;

        Add_Work([this, wProto, eLevel]() -> HRESULT
            {
                auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(wProto);
                if (!pReg) return E_FAIL;

                pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext, ETOUI(eLevel));
                m_pGameInstance_Proxy->Add_Prototype(ETOUI(eLevel), wProto.c_str(),
                    pReg->CreatorFunc(m_pDevice, m_pContext));
                return S_OK;
            });
    }

    if (!Manifest.strUIFile.empty())
    {
        wstring strUIFile = Manifest.strUIFile;
        Add_Work([this, strUIFile, eLevel]() -> HRESULT
            {
                return Ready_Level_UIResources(
                    m_pGameInstance_Proxy,
                    m_pDevice,
                    m_pContext,
                    strUIFile.c_str(),
                    ETOUI(eLevel));
            });
    }

    return S_OK;
}

HRESULT CLoader::Collect_And_StartWorkers()
{
    if (m_bNeedStatic)
        Ready_StaticResources();

    if (FAILED(Ready_WorkQueue()))
        return E_FAIL;

    const _uint iHW = thread::hardware_concurrency();
    m_iWorkerCount = std::clamp(iHW > 2 ? iHW - 2 : 1u, 1u, MAX_WORKER_COUNT);

    for (_uint i = 0; i < m_iWorkerCount; ++i)
    {
        m_hThreads[i] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr));
        if (0 == m_hThreads[i])
            return E_FAIL;
        SetThreadPriority(m_hThreads[i], THREAD_PRIORITY_BELOW_NORMAL);
    }

    m_bJobsReady = true;
    return S_OK;
}

HRESULT CLoader::Ready_StaticResources()
{
    Add_Work([this]() -> HRESULT
        {
            return Ready_Prototype_SharedResources(m_pGameInstance_Proxy, m_pDevice, m_pContext);
        }
    );

    Add_Work([this]() -> HRESULT
        {
            return Ready_Prototype_Shaders(m_pGameInstance_Proxy, m_pDevice, m_pContext);
        }
    );

    return S_OK;
}

HRESULT CLoader::Ready_WorkQueue()
{
    switch (m_eNextLevelID)
    {   
    case LEVEL::STATIC:
        break;
    case LEVEL::LOADING:
        break;
    case LEVEL::STAGE0_STEP1:
        if (FAILED(Read_Manifest(LAUNCHER_LEVEL_PROFILES::LEVEL_STAGE0_STEP1, LEVEL::STAGE0_STEP1)))
            return E_FAIL;
        break;
    case LEVEL::STAGE0_STEP2:
        if (FAILED(Read_Manifest(LAUNCHER_LEVEL_PROFILES::LEVEL_STAGE0_STEP2, LEVEL::STAGE0_STEP2)))
            return E_FAIL;
        break;
    case LEVEL::TEST:
        if (FAILED(Read_Manifest(LAUNCHER_LEVEL_PROFILES::LEVEL_TEST, LEVEL::TEST)))
            return E_FAIL;
        break;
    case LEVEL::TOWN_STEP1:
        if (FAILED(Read_Manifest(LAUNCHER_LEVEL_PROFILES::LEVEL_TOWN_STEP1, LEVEL::TOWN_STEP1)))
            return E_FAIL;
        break;
    case LEVEL::BOSS_STAGE1:
        if (FAILED(Read_Manifest(LAUNCHER_LEVEL_PROFILES::LEVEL_BOSS_STAGE1, LEVEL::BOSS_STAGE1)))
            return E_FAIL;
        break;
    case LEVEL::END:
        break;
    default:
        break;
    }
    return S_OK;
}

CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID, _bool Initialized)
{
    CLoader* pInstance = new CLoader(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevelID, Initialized)))
    {
        MSG_BOX("Failed to Created : CLoader");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader::Free()
{
    __super::Free();

    WaitForSingleObject(m_hCollector, INFINITE);
    CloseHandle(m_hCollector);

    if (m_iWorkerCount > 0)
    {
        WaitForMultipleObjects(m_iWorkerCount, m_hThreads, true, INFINITE);
        for (_uint i = 0; i < m_iWorkerCount; ++i)
            CloseHandle(m_hThreads[i]);
    }
    
    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
