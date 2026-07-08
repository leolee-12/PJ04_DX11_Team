#include "UI_MissionPanel.h"
#include "UIPartObject.h"
#include "UI_Text.h"

CUI_MissionPanel::CUI_MissionPanel(ID3D11Device* d, ID3D11DeviceContext* c) : CUI_GenericContainer{ d, c } {}
CUI_MissionPanel::CUI_MissionPanel(const CUI_MissionPanel& p) : CUI_GenericContainer(p) {}

HRESULT CUI_MissionPanel::Initialize_Prototype() { return __super::Initialize_Prototype(); }
HRESULT CUI_MissionPanel::Initialize(void* pArg) { return __super::Initialize(pArg); }

void CUI_MissionPanel::Set_Mission(_bool /*bIsMain*/, _bool bSucceeded, const _wstring& strName)
{
    m_bSucceeded = bSucceeded;

    if (auto it = m_UIPartObjects.find(PART_NAME); it != m_UIPartObjects.end())
        static_cast<CUI_Text*>(it->second)->Set_Text(strName);

    if (auto it = m_UIPartObjects.find(PART_STAMP); it != m_UIPartObjects.end())
        it->second->Set_Active(false);   // ¼º°ø¿¬Ãâ Àü±îÁö ½ºÅÆÇÁ ¼û±è
}

void CUI_MissionPanel::Play_Success()
{
    if (!m_bSucceeded) return;
    auto* pAnim = Get_UIAnimatorCom();
    auto it = m_UIPartObjects.find(PART_STAMP);
    if (!pAnim || it == m_UIPartObjects.end()) return;

    it->second->Set_Active(true);

    UI_FADE_DESC fd{}; fd.fFromAlpha = 0.f; fd.fToAlpha = -1.f; fd.fDuration = 0.15f;
    pAnim->Play_Fade(PART_STAMP, fd);

    UI_BOUNCE_DESC bd{};                 // ½ºÅÆÇÁ ÆË (ÇÊµå´Â ÇÁ·ÎÁ§Æ® UI_BOUNCE_DESC¿¡ ¸Â°Ô)
    pAnim->Play_Bounce(PART_STAMP, bd);
}

CUI_MissionPanel* CUI_MissionPanel::Create(ID3D11Device* d, ID3D11DeviceContext* c)
{
    auto* p = new CUI_MissionPanel(d, c);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_MissionPanel"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_MissionPanel::Clone(void* pArg)
{
    auto* p = new CUI_MissionPanel(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_MissionPanel"); Safe_Release(p); }
    return p;
}
void CUI_MissionPanel::Free() { __super::Free(); }