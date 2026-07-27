#include "Level_Loading.h"
#include "Loader.h"
#include "Launcher_LevelProfiles.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "GameObject.h"
#include "DataLoader.h"
#include "Loader_Prototype.h"

#include "Level_Test.h"

#include "Stage0_Step1.h"
#include "Stage0_Step2.h"

#include "Town_Step1.h"
#include "Town_Step2.h"

#include "Boss_Stage1.h"

#include "Stage1_Step1.h"
#include "Stage1_Step2.h"
#include "Stage1_Step3.h"

#include "Arena.h"
#include "Ending.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_eNextLevelID = eNextLevelID;

    if (FAILED(Load_LoadingLevel()))
        return E_FAIL;

    m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevelID);
    if (nullptr == m_pLoader)
        return E_FAIL;

    // FadeOut UI에서 BGM FadeOut을 호출함
    //m_pGameInstance_Proxy->Stop_BGM();
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
        case LEVEL::STAGE0_STEP1:
            pNextLevel = CStage0_Step1::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::STAGE0_STEP2:
            pNextLevel = CStage0_Step2::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::TEST:
            pNextLevel = CLevel_Test::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::TOWN_STEP1:
            pNextLevel = CTown_Step1::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::TOWN_STEP2:
            pNextLevel = CTown_Step2::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::BOSS_STAGE1:
            pNextLevel = CBoss_Stage1::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::STAGE1_STEP1:
            pNextLevel = CStage1_Step1::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::STAGE1_STEP2:
            pNextLevel = CStage1_Step2::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::STAGE1_STEP3:
            pNextLevel = CStage1_Step3::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::ARENA:
            pNextLevel = CArena::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::ENDING:
            pNextLevel = CEnding::Create(m_pDevice, m_pContext);
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
    const _tchar* szProfile = (LEVEL::ENDING == m_eNextLevelID)
        ? LAUNCHER_LEVEL_PROFILES::LEVEL_ENDINGLOADING
        : LAUNCHER_LEVEL_PROFILES::LEVEL_LOADING;
    
    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(szProfile, &Manifest)))
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
