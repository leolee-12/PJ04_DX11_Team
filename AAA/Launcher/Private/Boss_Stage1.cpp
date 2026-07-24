#include "Boss_Stage1.h"

#include "Level_Defines.h"

CBoss_Stage1::CBoss_Stage1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CBoss_Stage1::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::BOSS_STAGE1);

    LEVEL_LOAD_CONTEXT ctx{ m_pGameInstance_Proxy, m_pDevice, m_pContext };
    if (FAILED(Load_Level_FromManifest(ctx, LAUNCHER_LEVEL_PROFILES::LEVEL_BOSS_STAGE1, iLevel)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_PreBoss.marker.wav", 3.f, 0.15f);     // 1ÃÊ Fade IN µé¾îº¸¸ç Æ©´×

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
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::TOWN_STEP2);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }

    });

    Subscribe_Event(EventTag::BGMChange, [this](void* p) {
        m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_Boss1.marker.wav", 3.f, 0.25f);
        });    

    return S_OK;
};


HRESULT CBoss_Stage1::Ready_Lights()
{
    CLevelLight_DB::Apply(m_pGameInstance_Proxy, LEVEL::BOSS_STAGE1);

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
