#include "UI_TitleLogo.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"

CUI_TitleLogo::CUI_TitleLogo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_GenericContainer{ pDevice, pContext }
{
    m_fFadeInDur = { 2.f };
    m_fHoldDur = { 5.f };
    m_fFadeOutDur = { 1.f };
}

CUI_TitleLogo::CUI_TitleLogo(const CUI_TitleLogo& Prototype)
    : CUI_GenericContainer(Prototype)
    , m_fFadeInDur(Prototype.m_fFadeInDur)
    , m_fHoldDur(Prototype.m_fHoldDur)
    , m_fFadeOutDur(Prototype.m_fFadeOutDur)
{
}

HRESULT CUI_TitleLogo::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

HRESULT CUI_TitleLogo::Ready_Events()
{
    Subscribe_Event(EventTag::TitleLogo_Show, [this](void*) { Show(); });
    return S_OK;
}

void CUI_TitleLogo::On_Deserialized()
{
    __super::On_Deserialized();

    if (m_pGameInstance_Proxy && !m_pGameInstance_Proxy->Is_EditMode())
        Set_Active(false);
}

void CUI_TitleLogo::Show()
{
    if (m_eState != ELOGO::IDLE)
        return;

    Set_Active(true);

    UI_FADE_DESC Desc{};
    Desc.fFromAlpha = 0.f;
    Desc.fToAlpha = 1.f;
    Desc.fDuration = m_fFadeInDur;
    if (auto* pAnim = Get_UIAnimatorCom())
        pAnim->Play_FadeAll(Desc);

    m_eState = ELOGO::FADE_IN;
    m_fElapsed = 0.f;
}

void CUI_TitleLogo::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Update(fTimeDelta);

    if (m_eState == ELOGO::IDLE)
        return;

    m_fElapsed += fTimeDelta;

    switch (m_eState)
    {
        case ELOGO::FADE_IN:
            if (m_fElapsed >= m_fFadeInDur)
            {
                m_eState = ELOGO::HOLD;
                m_fElapsed = 0.f;
            }
            break;

        case ELOGO::HOLD:
            if (m_fElapsed >= m_fHoldDur)
            {
                UI_FADE_DESC Desc{};
                Desc.fFromAlpha = 1.f;
                Desc.fToAlpha = 0.f;
                Desc.fDuration = m_fFadeOutDur;
                if (auto* pAnim = Get_UIAnimatorCom())
                    pAnim->Play_FadeAll(Desc);

                m_eState = ELOGO::FADE_OUT;
                m_fElapsed = 0.f;
            }
            break;

        case ELOGO::FADE_OUT:
            if (m_fElapsed >= m_fFadeOutDur)
            {
                m_eState = ELOGO::IDLE;
                Set_Active(false);
            }
            break;
    }
}

CUI_TitleLogo* CUI_TitleLogo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_TitleLogo* pInstance = new CUI_TitleLogo(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_TitleLogo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_TitleLogo::Clone(void* pArg)
{
    CUI_TitleLogo* pInstance = new CUI_TitleLogo(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_TitleLogo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_TitleLogo::Free()
{
    __super::Free();
}