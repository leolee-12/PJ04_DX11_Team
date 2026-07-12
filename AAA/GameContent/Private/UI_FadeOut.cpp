#include "UI_FadeOut.h"
#include "GameInstance.h"

CUI_FadeOut::CUI_FadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainSequenceBase(pDevice, pContext)
{
}
CUI_FadeOut::CUI_FadeOut(const CUI_FadeOut& Prototype)
    : CUI_CurtainSequenceBase(Prototype) {
}

void CUI_FadeOut::On_SequenceStart()
{

    m_pGameInstance_Proxy->Play_SFX(L"UiResident_TownGeteIn.wav", 0.35f, ESoundBus::UI);
    m_pGameInstance_Proxy->Fade_BGM_Out(1.f);
}

void CUI_FadeOut::On_SequenceDone()
{
    m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Done"), nullptr);
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