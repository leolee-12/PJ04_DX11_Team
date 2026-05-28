#include "Level_Logo.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "Level_Loading.h"
#include "Level_GamePlay.h"
#include "Loader_Prototype.h"
#include "TitleUI.h"

CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel (pDevice, pContext)
{
}

HRESULT CLevel_Logo::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Load_Level(m_pGameInstance_Proxy, m_pDevice, m_pContext,
        L"../../Resources/LevelData/Title.JSON", ETOUI(LEVEL::LOGO))))
        return E_FAIL;

    m_pTitleUI_1 = m_pGameInstance_Proxy->Find_GameObject<CTitleUI>(ETOUI(LEVEL::LOGO), L"UI_Layer", L"Logo_1");
    m_pTitleUI_2 = m_pGameInstance_Proxy->Find_GameObject<CTitleUI>(ETOUI(LEVEL::LOGO), L"UI_Layer", L"Logo_2");

    m_pTitleUI_1->Start();

    return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{
    if (m_eLogoState == LOGO_STATE::DONE)
    {
        if (SUCCEEDED(m_pGameInstance_Proxy->Change_Level(ETOI(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOBBY))))
            return;
    }

    if (m_pGameInstance_Proxy->Key_Down(DIK_SPACE)) {
        if (!m_pTitleUI_2->Is_Done() && m_pTitleUI_1->Is_Done() == false) {
            m_pTitleUI_1->Skip();
            m_pTitleUI_2->Start();
            m_eLogoState = LOGO_STATE::LOGO2;
        }
        else if (m_pTitleUI_1->Is_Done() && !m_pTitleUI_2->Is_Done()) {
            m_pTitleUI_2->Skip();
            m_eLogoState = LOGO_STATE::DONE;
            return;
        }
    }

    if (m_pTitleUI_1->Is_Done() && m_eLogoState == LOGO_STATE::LOGO1)
    {
        m_pTitleUI_2->Start();
        m_eLogoState = LOGO_STATE::LOGO2;
    }

    if (m_pTitleUI_2->Is_Done() && m_eLogoState == LOGO_STATE::LOGO2)
        m_eLogoState = LOGO_STATE::DONE;
}

HRESULT CLevel_Logo::Render()
{
#ifdef _DEBUG

#endif
    return S_OK;
}

CLevel_Logo* CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Logo* pInstance = new CLevel_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Logo::Free()
{
    __super::Free();
}
