#include "Town_Step2.h"

#include "Level_Defines.h"

CTown_Step2::CTown_Step2(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CTown_Step2::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::TOWN_STEP2);

    LEVEL_LOAD_CONTEXT ctx{ m_pGameInstance_Proxy, m_pDevice, m_pContext };
    if (FAILED(Load_Level_FromManifest(ctx, LAUNCHER_LEVEL_PROFILES::LEVEL_TOWN_STEP2, iLevel)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Publish(EventTag::CutFade_Out, nullptr);
    m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_TownNewWorld1.marker.wav", 3.f, 0.2f);

    return S_OK;
}

void CTown_Step2::Update(_float fTimeDelta)
{
#ifdef  _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_F1))
        m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Start"), nullptr);
#endif //  _DEBUG
}

HRESULT CTown_Step2::Render()
{
    return S_OK;
}

HRESULT CTown_Step2::Ready_Events()
{
    Subscribe_Event(TEXT("FadeOut_Done"), [this](void* p) {
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::STAGE1_STEP1);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        });
    return S_OK;
}

HRESULT CTown_Step2::Ready_Lights()
{
    CLevelLight_DB::Apply(m_pGameInstance_Proxy, LEVEL::TOWN_STEP2);

    return S_OK;
}

CTown_Step2* CTown_Step2::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTown_Step2* pInstance = new CTown_Step2(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CTown_Step2");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTown_Step2::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
