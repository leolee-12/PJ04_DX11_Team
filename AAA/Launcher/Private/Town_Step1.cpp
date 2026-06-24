#include "Town_Step1.h"

#include "GameInstance.h"
#include "Camera_Free.h"
#include "Loader_Prototype.h"
#include "Map_Loader.h"
#include "Launcher_LevelProfiles.h"
#include "Camera_AreaCam.h"
#include "Level_Loading.h"

CTown_Step1::CTown_Step1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CTown_Step1::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::TOWN_STEP1);

    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_TOWN_STEP1, &Manifest)))
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

    if (FAILED(Ready_Camera()))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    return S_OK;
}

void CTown_Step1::Update(_float fTimeDelta)
{
#ifdef  _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_F1))
        m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Start"), nullptr);
#endif //  _DEBUG
}

HRESULT CTown_Step1::Render()
{
#ifdef _DEBUG
    SetWindowText(g_hWnd, TEXT("Å¸¿î STEP1."));
#endif
    return S_OK;
}

HRESULT CTown_Step1::Ready_Events()
{
    m_pGameInstance_Proxy->Subscribe(TEXT("FadeOut_Done"), [this](void* p) {
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BOSS_STAGE1);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        });
    return S_OK;
}

HRESULT CTown_Step1::Ready_Lights()
{
    LIGHT_DESC LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(5.0f, 5.5f, 6.0f, 1.f);
    LightDesc.vAmbient = _float4(0.f, 0.f, 0.f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(0.557f, -0.766f, 0.321f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CTown_Step1::Ready_Camera()
{
    m_pGameInstance_Proxy->Add_Prototype(ETOUI(LEVEL::TOWN_STEP1),
        TEXT("Prototype_GameObject_Camera_Follow"),
        CCamera_AreaCam::Create(m_pDevice, m_pContext));

    CCamera_AreaCam::AREACAM_DESC CamDesc{};
    CamDesc.vEye = _float3(-1.f, 1.f, -10.f);
    CamDesc.vAt = _float3(0.f, 0.f, 0.f);
    CamDesc.fFovy = XMConvertToRadians(50.f); CamDesc.fNear = 0.1f; CamDesc.fFar = 1000.f;
    CamDesc.strTargetLayer = TEXT("Layer_LiveObject");
    CamDesc.strTargetObj = TEXT("Proto_Kirby_0");
    CamDesc.strDataPath = TEXT("../../Resources/YSH/CameraData/Town_Step1_cam.json");
    m_pGameInstance_Proxy->Add_GameObject(ETOUI(LEVEL::TOWN_STEP1),
        TEXT("Prototype_GameObject_Camera_Follow"),
        ETOUI(LEVEL::TOWN_STEP1), TEXT("Layer_Camera"), TEXT("CameraFollow"), &CamDesc);

    return S_OK;
}

CTown_Step1* CTown_Step1::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTown_Step1* pInstance = new CTown_Step1(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CTown_Step1");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTown_Step1::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
