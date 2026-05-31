#include "Loader.h"
#include "GameInstance.h"
#include "Loader_Prototype.h"     // Client::Ready_Prototype_SharedResources / _Shaders / Load_Fonts

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
    CLoader* pLoader = static_cast<CLoader*>(pArg);

    if (FAILED(pLoader->Loading()))
        return -1;

    return 0;
}

HRESULT CLoader::Initialize(TOOL_LEVEL eNextLevelID)
{
    m_eNextLevelID = eNextLevelID;

    Ready_WorkQueue();

    for (_uint i = 0; i < WORKER_COUNT; ++i)
    {
        m_hThreads[i] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr));
        if (0 == m_hThreads[i])
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
            func();
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
    case TOOL_LEVEL::EDIT:
        Ready_Resources_For_Edit();
        break;
    default:
        break;
    }
    return S_OK;
}

HRESULT CLoader::Ready_Resources_For_Edit()
{
    // 에디터 진입에 필요한 공용 리소스/셰이더/폰트를 워커 스레드에서 적재한다.
    // (실제 배치 오브젝트의 프로토타입은 팩토리가 배치 시점에 온디맨드로 생성)
    Add_Work([this]() -> HRESULT
        {
            lstrcpy(m_szLoadingText, TEXT("Loading SharedResources..."));
            return Client::Ready_Prototype_SharedResources(m_pGameInstance_Proxy, m_pDevice, m_pContext);
        });

    Add_Work([this]() -> HRESULT
        {
            lstrcpy(m_szLoadingText, TEXT("Loading Shaders..."));
            return Client::Ready_Prototype_Shaders(m_pGameInstance_Proxy, m_pDevice, m_pContext);
        });

    Add_Work([this]() -> HRESULT
        {
            lstrcpy(m_szLoadingText, TEXT("Loading Fonts..."));
            return Client::Load_Fonts(m_pGameInstance_Proxy);
        });

    return S_OK;
}

CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TOOL_LEVEL eNextLevelID)
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
        CloseHandle(m_hThreads[i]);

    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
