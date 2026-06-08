#include "Loader.h"
#include "GameContent_const.h"
#include "GameInstance.h"
#include "GameObject_Factory.h"
#include "DataLoader.h"
#include "Map_Loader.h"
#include "Loader_Prototype.h"
#include "Launcher_MapProfiles.h"
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



HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
    m_eNextLevelID = eNextLevelID;

    Ready_WorkQueue();

    /* eNextLevelID에 필요한 자원을 로딩하는 작업을 수행한다. 누가? 스레드가 */
    //m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr));
    //if (0 == m_hThread)
    //    return E_FAIL;
    for (_uint i = 0; i < WORKER_COUNT; ++i)
    {
        m_hThreads[i] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr));
        if (0 == m_hThreads)
        {
            MSG_BOX("Create Failed : Loading Thread");
            return E_FAIL;
        }
    }

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
    SetWindowText(g_hWnd, m_szLoadingText);
}

#endif

void CLoader::Add_Work(function<HRESULT()>&& func)
{
    m_WorkQueue.push(move(func));
    ++m_iTotalWorkCount;
}

HRESULT CLoader::Ready_WorkQueue()
{
    switch (m_eNextLevelID)
    {   
    case Client::LEVEL::STATIC:
        break;
    case Client::LEVEL::LOADING:
        break;
    case Client::LEVEL::LOGO:
        break;
    case Client::LEVEL::LOBBY:
        Ready_Resources_For_Lobby();
        break;
    case Client::LEVEL::GAMEPLAY:
        Ready_Resources_For_GamePlay();
        break;
    case Client::LEVEL::END:
        break;
    default:
        break;
    }
    return E_NOTIMPL;
}

HRESULT CLoader::Ready_Resources_For_Lobby()
{
    string strContent;
    CDataLoader::Read_Json(L"../../Resources/LevelData/Lobby.JSON", &strContent);
    json jLevel = json::parse(strContent);

    LEVEL eLevel = LEVEL::LOBBY;

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

                pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);
                m_pGameInstance_Proxy->Add_Prototype(ETOUI(eLevel), wProto.c_str(),
                    pReg->CreatorFunc(m_pDevice, m_pContext));
                return S_OK;
            });
    }
    return S_OK;
}

HRESULT CLoader::Ready_Resources_For_GamePlay()
{
    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_TEST, &Manifest)))
        return E_FAIL;

    LEVEL eLevel = LEVEL::GAMEPLAY;

    Add_Work([this, Manifest, eLevel]() -> HRESULT
        {
            return CMap_Loader::Preload_Map(
                m_pDevice,
                m_pContext,
                Manifest.strMapManifest,
                ETOUI(eLevel));
        });

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

                pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);
                m_pGameInstance_Proxy->Add_Prototype(ETOUI(eLevel), wProto.c_str(),
                    pReg->CreatorFunc(m_pDevice, m_pContext));
                return S_OK;
            });
    }

    return S_OK;
}

CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
    CLoader* pInstance = new CLoader(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevelID)))
    {
        MSG_BOX("Failed to Created : CLoader");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader::Free()
{
    __super::Free();

    WaitForMultipleObjects(WORKER_COUNT, m_hThreads, true, INFINITE);
    for (_uint i = 0; i < WORKER_COUNT; ++i)
    {
        CloseHandle(m_hThreads[i]);
    }
    
    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
