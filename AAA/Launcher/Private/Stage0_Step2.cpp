#include "Stage0_Step2.h"

#include "Level_Defines.h"

CStage0_Step2::CStage0_Step2(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CStage0_Step2::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::STAGE0_STEP2);

    LEVEL_LOAD_CONTEXT ctx{ m_pGameInstance_Proxy, m_pDevice, m_pContext };
    if (FAILED(Load_Level_FromManifest(ctx, LAUNCHER_LEVEL_PROFILES::LEVEL_STAGE0_STEP2, iLevel)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_Grassland1.marker.wav", 5.f, 0.15f);

    return S_OK;
}

void CStage0_Step2::Update(_float fTimeDelta)
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

HRESULT CStage0_Step2::Render()
{
    return S_OK;
}

HRESULT CStage0_Step2::Ready_Events()
{
    Subscribe_Event(TEXT("FadeOut_Done"), [this](void* p) {
        //CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::TOWN_STEP1);
        //if (pLoadingLevel)
        //{
        //    m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
        //    return;
        //}
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BOSS_STAGE1);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        });
    return S_OK;
}

HRESULT CStage0_Step2::Ready_Lights()
{
    CLevelLight_DB::Apply(m_pGameInstance_Proxy, LEVEL::STAGE0_STEP2);

    return S_OK;
}

CStage0_Step2* CStage0_Step2::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStage0_Step2* pInstance = new CStage0_Step2(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CStage0_Step2");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStage0_Step2::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
