#include "Level_ArenaLoading.h"
#include "Loader.h"
#include "Launcher_LevelProfiles.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "Loader_Prototype.h"

#include "Arena.h"

CLevel_ArenaLoading::CLevel_ArenaLoading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_ArenaLoading::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Load_LoadingUI()))
        return E_FAIL;

    m_pLoader = CLoader::Create(m_pDevice, m_pContext, LEVEL::ARENA);
    if (nullptr == m_pLoader)
        return E_FAIL;

    m_pGameInstance_Proxy->Clear_Lights();

    return S_OK;
}

void CLevel_ArenaLoading::Update(_float fTimeDelta)
{
    if (m_bTransitioned)
        return;

    m_fElapsed += fTimeDelta;

    // 로딩바 등에서 쓰고 싶으면 이 값을 UI 로 밀어넣으면 됨
    _float fRatio = m_pLoader->Get_Progress();

    // 로딩 완료 + 커튼 연출 최소 노출 시간 경과 후 아레나로 전환
    if (m_pLoader->isFinished() && m_fElapsed >= MIN_DISPLAY_TIME)
    {
        CLevel* pNextLevel = CArena::Create(m_pDevice, m_pContext);
        if (nullptr == pNextLevel)
        {
            MSG_BOX("Failed to Changed : Arena");
            return;
        }

        if (SUCCEEDED(m_pGameInstance_Proxy->Change_Level(ETOI(LEVEL::ARENA), pNextLevel)))
        {
            m_bTransitioned = true;
            return;
        }
    }
}

HRESULT CLevel_ArenaLoading::Render()
{
#ifdef _DEBUG
    m_pLoader->Show();
#endif
    return S_OK;
}

HRESULT CLevel_ArenaLoading::Load_LoadingUI()
{
    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_ARENALOADING, &Manifest)))
        return E_FAIL;

    // 로딩 UI 는 LOADING 슬롯을 재사용 (동시에 로딩 레벨은 하나뿐)
    LEVEL eLevel = LEVEL::LOADING;

    if (!Manifest.strUIFile.empty())
    {
        wstring strUIFile = Manifest.strUIFile;
        if (FAILED(Ready_Level_UIResources(m_pGameInstance_Proxy, m_pDevice, m_pContext,
            strUIFile.c_str(), ETOUI(eLevel))))
            return E_FAIL;

        if (FAILED(Load_Level_UI(m_pGameInstance_Proxy, m_pDevice, m_pContext,
            Manifest.strUIFile.c_str(), ETOUI(eLevel))))
            return E_FAIL;
    }
    return S_OK;
}

CLevel_ArenaLoading* CLevel_ArenaLoading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_ArenaLoading* pInstance = new CLevel_ArenaLoading(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_ArenaLoading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_ArenaLoading::Free()
{
    __super::Free();
    Safe_Release(m_pLoader);
}