#include "Town_Step3.h"

#include "Level_Defines.h"

CTown_Step3::CTown_Step3(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CTown_Step3::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::TOWN_STEP3);

    LEVEL_LOAD_CONTEXT ctx{ m_pGameInstance_Proxy, m_pDevice, m_pContext };
    if (FAILED(Load_Level_FromManifest(ctx, LAUNCHER_LEVEL_PROFILES::LEVEL_TOWN_STEP3, iLevel)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Publish(EventTag::CutFade_Out, nullptr);

    return S_OK;
}

void CTown_Step3::Update(_float fTimeDelta)
{
#ifdef  _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_F1))
        m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Start"), nullptr);
#endif //  _DEBUG
}

HRESULT CTown_Step3::Render()
{
    return S_OK;
}

HRESULT CTown_Step3::Ready_Events()
{
    Subscribe_Event(TEXT("Arena_FadeOut_Done"), [this](void* p) {
        CLevel_ArenaLoading* pLoadingLevel = CLevel_ArenaLoading::Create(m_pDevice, m_pContext);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        });
    return S_OK;
}

HRESULT CTown_Step3::Ready_Lights()
{
    CLevelLight_DB::Apply(m_pGameInstance_Proxy, LEVEL::TOWN_STEP3);

    return S_OK;
}

CTown_Step3* CTown_Step3::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTown_Step3* pInstance = new CTown_Step3(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CTown_Step3");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTown_Step3::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
