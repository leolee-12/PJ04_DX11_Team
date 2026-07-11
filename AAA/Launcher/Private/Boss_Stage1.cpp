#include "Boss_Stage1.h"

#include "GameInstance.h"
#include "Loader_Prototype.h"
#include "Map_Loader.h"
#include "Launcher_LevelProfiles.h"
#include "Level_Loading.h"
#include "GameContrnt_Events.h"

CBoss_Stage1::CBoss_Stage1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CBoss_Stage1::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::BOSS_STAGE1);

    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_BOSS_STAGE1, &Manifest)))
        return E_FAIL;

    MAP_LOAD_RESULT MapReport{};
    CMapStage* pMapStage = nullptr;

    if (FAILED(CMap_Loader::Spawn_Map(
        m_pDevice,
        m_pContext,
        Manifest.strMapManifest,
        Manifest.strObjectsFile,
        iLevel,
        &MapReport,
        &pMapStage)))
    {
        return E_FAIL;
    }

    if (FAILED(Load_Level(m_pGameInstance_Proxy, m_pDevice, m_pContext,
        Manifest.strObjectsFile.c_str(), iLevel)))
        return E_FAIL;

#ifdef _DEBUG
    const _wstring strDebugMessage =
        L"[MapLoad][LevelDesign] json=" + to_wstring(MapReport.iLevelDesignJsonLoadedCount) +
        L", parsed=" + to_wstring(MapReport.iLevelDesignParsedObjectCount) +
        L", created=" + to_wstring(MapReport.iLevelDesignCreatedCount) +
        L", fallback=" + to_wstring(MapReport.iLevelDesignFallbackSpecCount) +
        L", failed=" + to_wstring(MapReport.iLevelDesignSkippedCreateFailedCount) + L"\n";

    OutputDebugStringW(strDebugMessage.c_str());
#endif

    if (!Manifest.strUIFile.empty())
    {
        if (FAILED(Load_Level_UI(m_pGameInstance_Proxy, m_pDevice, m_pContext,
            Manifest.strUIFile.c_str(), iLevel)))
            return E_FAIL;
    }

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_PreBoss.marker.wav", 3.f, 0.5f);     // 1ÃÊ Fade IN µé¾îº¸¸ç Æ©´×

    m_pGameInstance_Proxy->Set_ShaderGlobal("g_fFogEnable", _float4(0.f, 0.f, 0.f, 0.f));

    return S_OK;
}

void CBoss_Stage1::Update(_float fTimeDelta)
{
#ifdef  _DEBUG
    //if (m_pGameInstance_Proxy->Key_Down(DIK_F1))
    //    m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Start"), nullptr);
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
    {
        m_pGameInstance_Proxy->Publish(EventTag::Cutscene_GorillaAppear, nullptr);
        m_pGameInstance_Proxy->Toggle_DebugRender();
        //CUTSCENE_HANDOFF_DESC ho{};
        //m_pGameInstance_Proxy->Publish(EventTag::Cutscene_GorillaHandoff, &ho);
    }
#endif //  _DEBUG
}

HRESULT CBoss_Stage1::Render()
{
    return S_OK;
}

HRESULT CBoss_Stage1::Ready_Events()
{
    Subscribe_Event(TEXT("FadeOut_Done"), [this](void* p) {
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::TEST);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }

    });


    Subscribe_Event(EventTag::BGMChange, [this](void* p) {
        m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_Boss1.marker.wav", 3.f, 0.5f);
        });

    return S_OK;

    };


HRESULT CBoss_Stage1::Ready_Lights()
{
    LIGHT_DESC LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(6.2f, 6.15f, 5.76f, 1.f);
    //LightDesc.vAmbient = _float4(1.72f, 0.82f, 0.41f, 1.f);
    LightDesc.vAmbient = _float4(0.f, 0.f, 0.f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(0.557f, -0.766f, 0.321f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
        return E_FAIL;

    return S_OK;
}

CBoss_Stage1* CBoss_Stage1::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Stage1* pInstance = new CBoss_Stage1(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CBoss_Stage1");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBoss_Stage1::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
