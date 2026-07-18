#include "Stage1_Step1.h"

#include "Level_Defines.h"

CStage1_Step1::CStage1_Step1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CStage1_Step1::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::STAGE1_STEP1);

    LEVEL_LOAD_CONTEXT ctx{ m_pGameInstance_Proxy, m_pDevice, m_pContext };
    if (FAILED(Load_Level_FromManifest(ctx, LAUNCHER_LEVEL_PROFILES::LEVEL_STAGE1_STEP1, iLevel)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_Grassland1.marker.wav", 5.f, 0.15f);

    return S_OK;
}

void CStage1_Step1::Update(_float fTimeDelta)
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

HRESULT CStage1_Step1::Render()
{
    return S_OK;
}

HRESULT CStage1_Step1::Ready_Events()
{
    Subscribe_Event(TEXT("FadeOut_Done"), [this](void* p) {
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::STAGE1_STEP2);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        });
    return S_OK;
}

HRESULT CStage1_Step1::Ready_Lights()
{
    CLevelLight_DB::Apply(m_pGameInstance_Proxy, LEVEL::STAGE1_STEP1);

    return S_OK;
}

CStage1_Step1* CStage1_Step1::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStage1_Step1* pInstance = new CStage1_Step1(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CStage1_Step1");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStage1_Step1::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
