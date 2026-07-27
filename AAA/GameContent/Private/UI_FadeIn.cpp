#include "UI_FadeIn.h"
#include "GameInstance.h"

CUI_FadeIn::CUI_FadeIn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainSequenceBase(pDevice, pContext)
    , m_strDoneEvent{ L"FadeIn_Done" }
{
    m_bStartHidden = false;
    m_bAutoStart = true;
    m_bLoop = false;
}
CUI_FadeIn::CUI_FadeIn(const CUI_FadeIn& Prototype)
    : CUI_CurtainSequenceBase(Prototype) 
    , m_strDoneEvent{ Prototype.m_strDoneEvent }
{
}

void CUI_FadeIn::On_SequenceDone()
{
    Set_Active(false);
    if (!m_strDoneEvent.empty())
        m_pGameInstance_Proxy->Publish(m_strDoneEvent.c_str(), nullptr);
}

CUI_FadeIn* CUI_FadeIn::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_FadeIn* p = new CUI_FadeIn(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_FadeIn"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_FadeIn::Clone(void* pArg)
{
    CUI_FadeIn* p = new CUI_FadeIn(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_FadeIn"); Safe_Release(p); }
    return p;
}
void CUI_FadeIn::Free() { __super::Free(); }