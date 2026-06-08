#include "MainApp.h"

#include "GameInstance.h"
#include "Level_Loading.h"

#include "GameObject_Factory.h"
#include "Loader_Prototype.h"
#include "Level_Logo.h"

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

	if (FAILED(Ready_Prototype_For_Static()))
		return E_FAIL;

	CGameObject_Factory::GetInstance()->RegisterAll();


	if (FAILED(Load_Fonts(m_pGameInstance_Proxy)))
		return E_FAIL;

	if (FAILED(Start_Logo()))
		return E_FAIL;



	return S_OK;
}

void CMainApp::Update(_float fTimeDelta)
{
	m_pGameInstance_Proxy->Update_Engine(fTimeDelta);
}

HRESULT CMainApp::Render()
{
	if (FAILED(m_pGameInstance_Proxy->Begin_Draw()))
		return E_FAIL;

	if (FAILED(m_pGameInstance_Proxy->Draw()))
		return E_FAIL;

	if (FAILED(m_pGameInstance_Proxy->End_Draw()))
		return E_FAIL;

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

	CLevel* pLevelLoading = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY);
	if (nullptr == pLevelLoading)
		return E_FAIL;
	
	if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOI(LEVEL::LOADING), pLevelLoading)))
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

	Safe_Release(m_pGameInstance_Proxy);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	CGameInstance::DestroyInstance();

	CGameObject_Factory::DestroyInstance();
}
