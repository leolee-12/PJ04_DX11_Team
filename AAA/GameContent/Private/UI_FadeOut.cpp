#include "UI_FadeOut.h"
#include "GameInstance.h"

CUI_FadeOut::CUI_FadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainSequenceBase(pDevice, pContext)
    , m_strDoneEvent{ L"FadeOut_Done" }
    , m_strStartSFX{ L"UiResident_TownGeteIn.wav" }
    , m_fBGMFadeOut{ 2.f }
{
}
CUI_FadeOut::CUI_FadeOut(const CUI_FadeOut& Prototype)
    : CUI_CurtainSequenceBase(Prototype)
    , m_strDoneEvent{ Prototype.m_strDoneEvent }
    , m_strStartSFX{ Prototype. m_strStartSFX }
    , m_fBGMFadeOut{ Prototype.m_fBGMFadeOut }
{
}

void CUI_FadeOut::On_SequenceStart()
{
    if (!m_strStartSFX.empty())
        m_pGameInstance_Proxy->Play_SFX(m_strStartSFX.c_str(), 0.35f, ESoundBus::UI);

    if (m_fBGMFadeOut > 0.f)
        m_pGameInstance_Proxy->Fade_BGM_Out(m_fBGMFadeOut);
}

void CUI_FadeOut::On_SequenceDone()
{
    if (!m_strDoneEvent.empty())
        m_pGameInstance_Proxy->Publish(m_strDoneEvent.c_str(), nullptr);
}

void CUI_FadeOut::On_Event(void* pData)
{
    if (FADEOUT_PAYLOAD* pPayload = static_cast<FADEOUT_PAYLOAD*>(pData))
    {
        m_fBGMFadeOut = pPayload->fSFXDuration;
    }
}

CUI_FadeOut* CUI_FadeOut::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_FadeOut* p = new CUI_FadeOut(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_FadeOut"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_FadeOut::Clone(void* pArg)
{
    CUI_FadeOut* p = new CUI_FadeOut(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_FadeOut"); Safe_Release(p); }
    return p;
}
void CUI_FadeOut::Free() { __super::Free(); }