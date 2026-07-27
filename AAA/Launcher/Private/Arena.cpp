#include "Arena.h"

#include "Level_Defines.h"

CArena::CArena(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CArena::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::ARENA);

    LEVEL_LOAD_CONTEXT ctx{ m_pGameInstance_Proxy, m_pDevice, m_pContext };
    if (FAILED(Load_Level_FromManifest(ctx, LAUNCHER_LEVEL_PROFILES::LEVEL_ARENA, iLevel)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Play_BGM_Fade(L"Ec_MetaWind0.wav", 5.f, 0.1f);

    return S_OK;
}

void CArena::Update(_float fTimeDelta)
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

HRESULT CArena::Render()
{
    return S_OK;
}

HRESULT CArena::Ready_Events()
{
    Subscribe_Event(TEXT("FadeOut_Done"), [this](void* p) {
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::ENDING);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
        });

    Subscribe_Event(EventTag::Kirby_PositionSyncBegin, [this](void* pData) {
        const auto* pDesc = static_cast<KIRBY_POSITION_SYNC_BEGIN_DESC*>(pData);
        if (nullptr == pDesc)
            return;
        if (pDesc->eType != KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_LOOKAROUND)
            return;

        m_pGameInstance_Proxy->Play_BGM_Fade(L"K15_BossMetaknight1.marker.wav", 1.f, 0.15f);

        });
    return S_OK;
}

HRESULT CArena::Ready_Lights()
{
    CLevelLight_DB::Apply(m_pGameInstance_Proxy, LEVEL::ARENA);

    return S_OK;
}

CArena* CArena::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CArena* pInstance = new CArena(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CArena");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CArena::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
