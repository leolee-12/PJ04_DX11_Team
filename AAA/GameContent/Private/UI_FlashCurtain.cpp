#include "UI_FlashCurtain.h"
#include "GameInstance.h"

CUI_FlashCurtain::CUI_FlashCurtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainSequenceBase(pDevice, pContext)
{
    m_bStartHidden = true;   
    m_bAutoStart = false;   
    m_bLoop = false;   
}
CUI_FlashCurtain::CUI_FlashCurtain(const CUI_FlashCurtain& Prototype)
    : CUI_CurtainSequenceBase(Prototype) {
}

void CUI_FlashCurtain::On_SequenceDone()
{
    Set_Active(false);   
    m_pGameInstance_Proxy->Publish(TEXT("FlashCurtain_Done"), nullptr);
}

CUI_FlashCurtain* CUI_FlashCurtain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_FlashCurtain* p = new CUI_FlashCurtain(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_FlashCurtain"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_FlashCurtain::Clone(void* pArg)
{
    CUI_FlashCurtain* p = new CUI_FlashCurtain(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_FlashCurtain"); Safe_Release(p); }
    return p;
}
void CUI_FlashCurtain::Free() { __super::Free(); }