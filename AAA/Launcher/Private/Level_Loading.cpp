#include "Level_Loading.h"
#include "Loader.h"
#include "Launcher_LevelProfiles.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "GameObject.h"
#include "DataLoader.h"
#include "Loader_Prototype.h"

#include "Level_GamePlay.h"
#include "Level_Test.h"
#include "Town_Step1.h"
#include "Boss_Stage1.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Load_LoadingLevel()))
        return E_FAIL;

    m_eNextLevelID = eNextLevelID;

    m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevelID);
    if (nullptr == m_pLoader)
        return E_FAIL;

    m_pGameInstance_Proxy->Stop_BGM();
    m_pGameInstance_Proxy->Clear_Lights();

    return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{
    _float fRatio = m_pLoader->Get_Progress();

    if (true == m_pLoader->isFinished())
    {
        CLevel* pNextLevel = { nullptr };

        switch (m_eNextLevelID)
        {
        case LEVEL::GAMEPLAY:
            pNextLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::TEST:
            pNextLevel = CLevel_Test::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::TOWN_STEP1:
            pNextLevel = CTown_Step1::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::BOSS_STAGE1:
            pNextLevel = CBoss_Stage1::Create(m_pDevice, m_pContext);
            break;
        }

        if (nullptr == pNextLevel)
        {
            MSG_BOX("Failed to Changed");
            return;
        }

        if (SUCCEEDED(m_pGameInstance_Proxy->Change_Level(ETOI(m_eNextLevelID), pNextLevel)))
            return;
    }
}

HRESULT CLevel_Loading::Render()
{
#ifdef _DEBUG
    m_pLoader->Show();
#endif
    return S_OK;
}

HRESULT CLevel_Loading::Load_LoadingLevel()
{
    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_LOADING, &Manifest)))
        return E_FAIL;

    LEVEL eLevel = LEVEL::LOADING;

    if (!Manifest.strUIFile.empty())
    {
        wstring strUIFile = Manifest.strUIFile;
        if (FAILED(Ready_Level_UIResources(m_pGameInstance_Proxy, m_pDevice, m_pContext, strUIFile.c_str(), ETOUI(eLevel))))
            return E_FAIL;

        if (FAILED(Load_Level_UI(m_pGameInstance_Proxy, m_pDevice, m_pContext,
            Manifest.strUIFile.c_str(), ETOUI(eLevel))))
            return E_FAIL;
    }
    return S_OK;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
    CLevel_Loading* pInstance = new CLevel_Loading(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevelID)))
    {
        MSG_BOX("Failed to Created : CLevel_Loading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Loading::Free()
{
    __super::Free();
    Safe_Release(m_pLoader);
}
