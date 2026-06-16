#include "UI_LoadingCurtain.h"

CUI_LoadingCurtain::CUI_LoadingCurtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainSequenceBase(pDevice, pContext)
{
    m_bAutoStart = true;
}
CUI_LoadingCurtain::CUI_LoadingCurtain(const CUI_LoadingCurtain& Prototype)
    : CUI_CurtainSequenceBase(Prototype) {
}

CUI_LoadingCurtain* CUI_LoadingCurtain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_LoadingCurtain* p = new CUI_LoadingCurtain(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_LoadingCurtain"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_LoadingCurtain::Clone(void* pArg)
{
    CUI_LoadingCurtain* p = new CUI_LoadingCurtain(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_LoadingCurtain"); Safe_Release(p); }
    return p;
}
void CUI_LoadingCurtain::Free() { __super::Free(); }