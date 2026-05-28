#include "Level_Lobby.h"

#include "GameInstance.h"
#include "Camera_Free.h"
#include "Loader_Prototype.h"
#include "Level_Loading.h"

CLevel_Lobby::CLevel_Lobby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Lobby::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Load_Level(m_pGameInstance_Proxy, m_pDevice, m_pContext,
        L"../../Resources/LevelData/Lobby.JSON", ETOUI(LEVEL::LOBBY))))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    m_pGameInstance_Proxy->Play_BGM(TEXT("BGM_Lobby_0.wav"), ETOUI(SOUND_CHANNEL::BGM), 1.f);

    return S_OK;
}

void CLevel_Lobby::Update(_float fTimeDelta)
{
    if (m_bLevelChange_GameStart)
    {
        if (FAILED(m_pGameInstance_Proxy->Change_Level(ETOI(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY))))
        {
            MSG_BOX("Level Change Failed : GamePlay");
        }
        return;
    }

    if (m_pGameInstance_Proxy->Key_Down(DIK_ESCAPE))
    {
        if (!m_bInGameStart)
            m_pGameInstance_Proxy->Publish(TEXT("Return_Lobby"), nullptr);
    }
}

HRESULT CLevel_Lobby::Render()
{
#ifdef _DEBUG
    SetWindowText(g_hWnd, TEXT("·Îºñ."));
#endif
    return S_OK;
}

HRESULT CLevel_Lobby::Ready_Events()
{
    Subscribe_Event(TEXT("Enter_View"), [this](void* pData) {
        _int iType = *(static_cast<_int*>(pData));
        m_bInGameStart = (iType == ETOI(LOBBY_VIEW::GAME_START));
        });

    Subscribe_Event(TEXT("Return_Lobby"), [this](void* pData) {
        m_bInGameStart = false;
        });

    Subscribe_Event(TEXT("Click_GameStartBtn"), [this](void* pData) {
        if (!m_bInGameStart) return;
        m_bLevelChange_GameStart = true;
        });

    return S_OK;
}

HRESULT CLevel_Lobby::Ready_Lights()
{
    LIGHT_DESC      LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vAmbient = _float4(0.8f, 0.8f, 0.8f, 1.f);
    LightDesc.vSpecular = _float4(0.f, 0.f, 0.f, 1.f);
    LightDesc.vDirection = _float4(0.5f, 0.f, 1.f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
        return E_FAIL;

    SHADOW_LIGHT_DESC      ShadowLightDesc{};

    ShadowLightDesc.fFar = 50.f;
    ShadowLightDesc.fNear = 0.1f;
    ShadowLightDesc.fHeight = 10.f;
    ShadowLightDesc.fWidth = 10.f;
    ShadowLightDesc.vEye = _float4(139.f, 5.f, 83.f, 1.f);
    ShadowLightDesc.vAt = _float4(149.f, 0.22f, 83.f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_ShadowLight(ShadowLightDesc)))
        return E_FAIL;

    return S_OK;
}

CLevel_Lobby* CLevel_Lobby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Lobby* pInstance = new CLevel_Lobby(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_Lobby");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Lobby::Free()
{
    __super::Free();
}
