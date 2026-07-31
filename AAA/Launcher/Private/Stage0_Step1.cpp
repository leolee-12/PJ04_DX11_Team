#include "Stage0_Step1.h"

#include "Level_Defines.h"

#ifdef _DEBUG
namespace
{
    LEVEL Resolve_DebugLevel(const _wstring& strLevelTag)
    {
        if (strLevelTag == L"STAGE0_STEP1") return LEVEL::STAGE0_STEP1;
        if (strLevelTag == L"STAGE0_STEP2") return LEVEL::STAGE0_STEP2;
        if (strLevelTag == L"TOWN_STEP1") return LEVEL::TOWN_STEP1;
        if (strLevelTag == L"TOWN_STEP2") return LEVEL::TOWN_STEP2;
        if (strLevelTag == L"TOWN_STEP3") return LEVEL::TOWN_STEP3;
        if (strLevelTag == L"BOSS_STAGE1") return LEVEL::BOSS_STAGE1;
        if (strLevelTag == L"STAGE1_STEP1") return LEVEL::STAGE1_STEP1;
        if (strLevelTag == L"STAGE1_STEP2") return LEVEL::STAGE1_STEP2;
        if (strLevelTag == L"STAGE1_STEP3") return LEVEL::STAGE1_STEP3;
        if (strLevelTag == L"ENDING") return LEVEL::ENDING;
        if (strLevelTag == L"TEST") return LEVEL::TEST;
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

    LEVEL_LOAD_CONTEXT ctx{ m_pGameInstance_Proxy, m_pDevice, m_pContext };
    if (FAILED(Load_Level_FromManifest(ctx, LAUNCHER_LEVEL_PROFILES::LEVEL_STAGE0_STEP1, iLevel)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;


    return S_OK;
}

void CStage0_Step1::Update(_float fTimeDelta)
{
#ifdef  _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_F1))
    {
        m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Start"), nullptr);
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
    {
        m_pGameInstance_Proxy->Toggle_DebugRender();
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_F3))
        m_pGameInstance_Proxy->Publish(TEXT("EndingFade_Start"), nullptr);
    if (m_pGameInstance_Proxy->Key_Down(DIK_F4))
        m_pGameInstance_Proxy->Publish(TEXT("Arena_FadeOut_Start"), nullptr);
    
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
        });

    Subscribe_Event(TEXT("Arena_FadeOut_Done"), [this](void* p) {
        CLevel_ArenaLoading* pLoadingLevel = CLevel_ArenaLoading::Create(m_pDevice, m_pContext);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
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
    CLevelLight_DB::Apply(m_pGameInstance_Proxy, LEVEL::STAGE0_STEP1);

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
