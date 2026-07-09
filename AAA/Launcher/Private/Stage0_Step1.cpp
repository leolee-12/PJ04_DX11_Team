#include "Stage0_Step1.h"

#include "GameInstance.h"
#include "Camera_Free.h"
#include "Loader_Prototype.h"
#include "Map_Loader.h"
#include "Launcher_LevelProfiles.h"
#include "Camera_AreaCam.h"
#include "Level_Loading.h"

#ifdef _DEBUG
namespace
{
    LEVEL Resolve_DebugLevel(const _wstring& strLevelTag)
    {
        if (strLevelTag == L"STAGE0_STEP1" || strLevelTag == L"Stage0_Step1" || strLevelTag == L"Stage1-1") return LEVEL::STAGE0_STEP1;
        if (strLevelTag == L"STAGE0_STEP2" || strLevelTag == L"Stage0_Step2" || strLevelTag == L"Stage1-2") return LEVEL::STAGE0_STEP2;
        if (strLevelTag == L"TOWN_STEP1" || strLevelTag == L"Town_Step1") return LEVEL::TOWN_STEP1;
        if (strLevelTag == L"BOSS_STAGE1" || strLevelTag == L"Boss_Stage1" || strLevelTag == L"BossMap1") return LEVEL::BOSS_STAGE1;
        if (strLevelTag == L"TEST" || strLevelTag == L"Test") return LEVEL::TEST;
        return LEVEL::END;
    }
}
#endif

CStage0_Step1::CStage0_Step1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CStage0_Step1::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::STAGE0_STEP1);

    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_STAGE0_STEP1, &Manifest)))
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

void CStage0_Step1::Update(_float fTimeDelta)
{
#ifdef  _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_F1))
        m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Start"), nullptr);
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
    {
        m_pGameInstance_Proxy->Toggle_DebugRender();
    }
#endif //  _DEBUG
}

HRESULT CStage0_Step1::Render()
{
    return S_OK;
}

HRESULT CStage0_Step1::Ready_Events()
{
    Subscribe_Event(TEXT("FadeOut_Done"), [this](void* p) {
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::STAGE0_STEP2);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        //CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BOSS_STAGE1);
        //if (pLoadingLevel)
        //{
        //    m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
        //    return;
        //}
        });

#ifdef _DEBUG
    Subscribe_Event(TEXT("Debug Level Change"), [this](void* p){
        const TRIGGER_EVENT_PAYLOAD* pPayload = static_cast<const TRIGGER_EVENT_PAYLOAD*>(p);
        if (nullptr == pPayload)
        {
            OutputDebugStringW(L"[Stage0_Step1] Debug Level Change payload is null.\n");
            return;
        }

        const LEVEL eNextLevel = Resolve_DebugLevel(pPayload->strPayload);
        if (LEVEL::END == eNextLevel)
        {
            const _wstring strLog = L"[Stage0_Step1] Unknown debug level payload: " + pPayload->strPayload + L"\n";
            OutputDebugStringW(strLog.c_str());
            return;
        }

        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, eNextLevel);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        });
#endif

    return S_OK;
}

HRESULT CStage0_Step1::Ready_Lights()
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

HRESULT CStage0_Step1::Ready_Camera()
{
    m_pGameInstance_Proxy->Add_Prototype(ETOUI(LEVEL::STAGE0_STEP1),
        TEXT("Prototype_GameObject_Camera_Follow"),
        CCamera_AreaCam::Create(m_pDevice, m_pContext));

    CCamera_AreaCam::AREACAM_DESC CamDesc{};
    CamDesc.vEye = _float3(-1.f, 1.f, -10.f);
    CamDesc.vAt = _float3(0.f, 0.f, 0.f);
    CamDesc.fFovy = XMConvertToRadians(50.f); CamDesc.fNear = 0.1f; CamDesc.fFar = 1000.f;
    m_pGameInstance_Proxy->Add_GameObject(ETOUI(LEVEL::STAGE0_STEP1),
        TEXT("Prototype_GameObject_Camera_Follow"),
        ETOUI(LEVEL::STAGE0_STEP1), TEXT("Layer_Camera"), TEXT("CameraFollow"), &CamDesc);



    return S_OK;
}

CStage0_Step1* CStage0_Step1::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStage0_Step1* pInstance = new CStage0_Step1(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CStage0_Step1");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStage0_Step1::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
