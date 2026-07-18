#include "UI_CutFade.h"
#include "GameInstance.h"
#include "GameContent_Events.h"

CUI_CutFade::CUI_CutFade(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_GenericContainer{ pDevice, pContext }
{
}

CUI_CutFade::CUI_CutFade(const CUI_CutFade& Prototype)
    : CUI_GenericContainer(Prototype)
    , m_fOutDur(Prototype.m_fOutDur)
    , m_fInDur(Prototype.m_fInDur)
{
}

HRESULT CUI_CutFade::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

HRESULT CUI_CutFade::Ready_Events()
{
    Subscribe_Event(EventTag::CutFade_Out, [this](void*) { Begin_FadeOut(); });
    Subscribe_Event(EventTag::CutFade_In, [this](void*) { Begin_FadeIn(); });
    return S_OK;
}

void CUI_CutFade::On_Deserialized()
{
    __super::On_Deserialized();

    if (m_pGameInstance_Proxy && !m_pGameInstance_Proxy->Is_EditMode())
        Set_Active(false);
}

void CUI_CutFade::Begin_FadeOut()
{
    if (EFADE::IDLE != m_eState)
        return;

    Set_Active(true);

    UI_FADE_DESC Desc{};
    Desc.fFromAlpha = 0.f;
    Desc.fToAlpha = 1.f;
    Desc.fDuration = m_fOutDur;
    if (auto* pAnim = Get_UIAnimatorCom())
        pAnim->Play_FadeAll(Desc);

    m_eState = EFADE::FADEOUT;
    m_fElapsed = 0.f;
}

void CUI_CutFade::Begin_FadeIn()
{
    if (EFADE::BLACK != m_eState)
        return;

    UI_FADE_DESC Desc{};
    Desc.fFromAlpha = 1.f;
    Desc.fToAlpha = 0.f;
    Desc.fDuration = m_fInDur;
    if (auto* pAnim = Get_UIAnimatorCom())
        pAnim->Play_FadeAll(Desc);

    m_eState = EFADE::FADEIN;
    m_fElapsed = 0.f;
}

void CUI_CutFade::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Update(fTimeDelta);

    if (EFADE::FADEOUT != m_eState && EFADE::FADEIN != m_eState)
        return;

    m_fElapsed += fTimeDelta;

    if (EFADE::FADEOUT == m_eState && m_fElapsed >= m_fOutDur)
    {
        // 완전 암전 도달. In 신호 올 때까지 검은 화면 유지
        m_eState = EFADE::BLACK;
        m_pGameInstance_Proxy->Publish(EventTag::CutFade_OutDone, nullptr);
    }
    else if (EFADE::FADEIN == m_eState && m_fElapsed >= m_fInDur)
    {
        m_eState = EFADE::IDLE;
        Set_Active(false);
        m_pGameInstance_Proxy->Publish(EventTag::CutFade_InDone, nullptr);
    }
}

CUI_CutFade* CUI_CutFade::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_CutFade* pInstance = new CUI_CutFade(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_CutFade");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_CutFade::Clone(void* pArg)
{
    CUI_CutFade* pInstance = new CUI_CutFade(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_CutFade");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_CutFade::Free()
{
    __super::Free();
}