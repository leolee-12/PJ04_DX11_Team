#include "MainApp.h"

#include "GameInstance.h"

#include "GameObject_Factory.h"
#include "Loader_Prototype.h"
#include "Level_FirstLoading.h"
#include "Singleton_Destroyer.h"

// MainApp.cpp - 히칭 구간 계측용 (원인 확인 후 제거)
static _float s_fUpdateMs = 0.f;

CMainApp::CMainApp()
{
}

HRESULT CMainApp::Initialize()
{
	ENGINE_DESC         EngineDesc{};
	EngineDesc.hInstance = g_hInstance;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.eWinMode = WINMODE::WIN;
	EngineDesc.iViewportWidth = g_iWinSizeX;
	EngineDesc.iViewportHeight = g_iWinSizeY;
	EngineDesc.iNumLevels = ETOUI(LEVEL::END);

	if (FAILED(CGameInstance::Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
	{
		MSG_BOX("Failed to Initialize : Engine");
		return E_FAIL;
	}

	m_pGameInstance_Proxy = CGameInstance::GetProxy();
	m_pGameInstance_Proxy->Enable_InputDeveice();

	CGameObject_Factory::GetInstance()->RegisterAll();

	if (FAILED(Load_Fonts(m_pGameInstance_Proxy)))
		return E_FAIL;

	if (FAILED(Start_Logo()))
		return E_FAIL;

	return S_OK;
}

void CMainApp::Update(_float fTimeDelta, _float fRawTimeDelta)
{
	LARGE_INTEGER f, a, b;
	QueryPerformanceFrequency(&f);
	QueryPerformanceCounter(&a);
	m_pGameInstance_Proxy->Update_Engine(fTimeDelta, fRawTimeDelta);
	QueryPerformanceCounter(&b);
	s_fUpdateMs = (b.QuadPart - a.QuadPart) * 1000.f / f.QuadPart;
}

HRESULT CMainApp::Render()
{
	LARGE_INTEGER f, t0, t1, t2, t3;
	QueryPerformanceFrequency(&f);
	auto Ms = [&](LONGLONG a, LONGLONG b) { return (b - a) * 1000.f / f.QuadPart; };

	QueryPerformanceCounter(&t0);
	if (FAILED(m_pGameInstance_Proxy->Begin_Draw()))  return E_FAIL;
	QueryPerformanceCounter(&t1);
	if (FAILED(m_pGameInstance_Proxy->Draw()))        return E_FAIL;
	QueryPerformanceCounter(&t2);
	if (FAILED(m_pGameInstance_Proxy->End_Draw()))    return E_FAIL;   // Present
	QueryPerformanceCounter(&t3);

	if (s_fUpdateMs + Ms(t0.QuadPart, t3.QuadPart) > 25.f)
	{
		char buf[160];
		sprintf_s(buf, "[Hitch] update=%.1f begin=%.1f draw=%.1f present=%.1f\n",
			s_fUpdateMs, Ms(t0.QuadPart, t1.QuadPart), Ms(t1.QuadPart, t2.QuadPart), Ms(t2.QuadPart, t3.QuadPart));
		OutputDebugStringA(buf);
	}
	return S_OK;

	return S_OK;
}

HRESULT CMainApp::Ready_Prototype_For_Static()
{
	if (FAILED(Ready_Prototype_SharedResources(m_pGameInstance_Proxy, m_pDevice, m_pContext)))
	{
		MSG_BOX("Load Failed : SharedResources");
		return E_FAIL;
	}

	if (FAILED(Ready_Prototype_Shaders(m_pGameInstance_Proxy, m_pDevice, m_pContext)))
	{
		MSG_BOX("Load Failed : Shaders");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMainApp::Start_Logo()
{
	//CLevel* pLevelLogo = CLevel_Logo::Create(m_pDevice, m_pContext);
	//if (nullptr == pLevelLogo)
	//	return E_FAIL;
	//
	//if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOI(LEVEL::LOGO), pLevelLogo)))
	//	return E_FAIL;

	CLevel* pLevelLoading = CLevel_FirstLoading::Create(m_pDevice, m_pContext, LEVEL::STAGE0_STEP1);
	if (nullptr == pLevelLoading)
		return E_FAIL;
	
	if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOI(LEVEL::FIRST_LOADING), pLevelLoading)))
		return E_FAIL;

	return S_OK;
}

CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMainApp::Free()
{
	__super::Free();

	CSingleton_Destroyer::Destroy_GameContent_Singletons();

	Safe_Release(m_pGameInstance_Proxy);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	CGameInstance::DestroyInstance();
}
