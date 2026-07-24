#include "UI_UIFadeOut.h"
#include "GameInstance.h"
#include "UIAnimatorCom.h"

CUI_UIFadeOut::CUI_UIFadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_GenericContainer(pDevice, pContext), m_fFadeDelay{ 0.f }, m_fFadeDuration{ 0.5f } {
}
CUI_UIFadeOut::CUI_UIFadeOut(const CUI_UIFadeOut& Prototype)
    : CUI_GenericContainer(Prototype), m_strTriggerEvent{ Prototype.m_strTriggerEvent }
    , m_fFadeDelay{ Prototype.m_fFadeDelay }, m_fFadeDuration{ Prototype.m_fFadeDuration } {
}

HRESULT CUI_UIFadeOut::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))   // GenericContainer: UI 애니메이터 생성 + Bind_Parts
        return E_FAIL;
    m_bFading = false; m_bDone = false;
    return S_OK;
}

void CUI_UIFadeOut::On_Deserialized()
{
    __super::On_Deserialized();   // Bind_UIAnimator
    Refresh_Subscription();
}

void CUI_UIFadeOut::Refresh_Subscription()
{
    if (m_bSubscribed && m_hTrigger.strEventType != m_strTriggerEvent)
    {
        m_pGameInstance_Proxy->UnSubscribe(m_hTrigger);
        m_bSubscribed = false;
    }
    if (!m_strTriggerEvent.empty() && !m_bSubscribed)
    {
        m_hTrigger = m_pGameInstance_Proxy->Subscribe(m_strTriggerEvent,
            [this](void*) {
                if (m_bFading) return;
                Start_FadeOut();
                m_bFading = true; m_bDone = false;
            });
        m_bSubscribed = true;
    }
}

void CUI_UIFadeOut::Start_FadeOut()
{
    CUIAnimatorCom* pAnim = Get_UIAnimatorCom();
    if (!pAnim) return;

    UI_FADE_DESC d{};
    d.fFromAlpha = -1.f;             // 현재(베이스) 알파에서
    d.fToAlpha = 0.f;              // 0으로 (페이드아웃)
    d.fDuration = m_fFadeDuration;
    d.fDelay = m_fFadeDelay;     // 내장 딜레이
    d.bRestoreOnFinish = false;
    pAnim->Play_FadeAll(d);
}

void CUI_UIFadeOut::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);   // GenericContainer: 애니메이터 Update + 자식 Update

    if (m_bFading && !m_bDone)
    {
        CUIAnimatorCom* pAnim = Get_UIAnimatorCom();
        if (pAnim && !pAnim->Is_FadingAny())
        {
            m_bDone = true;
            m_pGameInstance_Proxy->Publish(TEXT("UIFadeOut_Done"), nullptr);
            Set_Active(false);
        }
    }
}

CUI_UIFadeOut* CUI_UIFadeOut::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_UIFadeOut* p = new CUI_UIFadeOut(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_UIFadeOut"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_UIFadeOut::Clone(void* pArg)
{
    CUI_UIFadeOut* p = new CUI_UIFadeOut(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_UIFadeOut"); Safe_Release(p); }
    return p;
}
void CUI_UIFadeOut::Free()
{
    if (m_bSubscribed) { m_pGameInstance_Proxy->UnSubscribe(m_hTrigger); m_bSubscribed = false; }
    __super::Free();
}