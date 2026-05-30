#include "Loader.h"
#include "GameContent_const.h"
#include "GameInstance.h"

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

HRESULT CLoader::Initialize(EDIT_LEVEL eNextLevelID)
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
    CoInitializeEx(nullptr, 0);

    while (true)
    {
        function<HRESULT()> func;
        if (m_WorkQueue.try_pop(func))
        {
            if (SUCCEEDED(func()))
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
    SetWindowText(g_hWndEditor, m_szLoadingText);
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
    case EDIT_LEVEL::EDIT:
        break;
    }
    return S_OK;
}

HRESULT CLoader::Ready_Resources_For_Logo()
{
    lstrcpy(m_szLoadingText, TEXT("텍스쳐 로딩 중"));

    lstrcpy(m_szLoadingText, TEXT("셰이더 로딩 중"));


    lstrcpy(m_szLoadingText, TEXT("정점, 인덱스 버퍼 로딩 중"));


    lstrcpy(m_szLoadingText, TEXT("객체원형 로딩 중"));


    lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

    return S_OK;
}

HRESULT CLoader::Ready_Resources_For_GamePlay()
{
    lstrcpy(m_szLoadingText, TEXT("텍스쳐 로딩 중"));


    lstrcpy(m_szLoadingText, TEXT("셰이더 로딩 중"));


    lstrcpy(m_szLoadingText, TEXT("정점, 인덱스 버퍼 로딩 중"));


    lstrcpy(m_szLoadingText, TEXT("객체원형 로딩 중"));


    lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

    return S_OK;
}

CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, EDIT_LEVEL eNextLevelID)
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
