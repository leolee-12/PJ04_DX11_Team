#include "UI_MissionPanel.h"
#include "UIPartObject.h"
#include "UI_Text.h"

CUI_MissionPanel::CUI_MissionPanel(ID3D11Device* d, ID3D11DeviceContext* c) : CUI_GenericContainer{ d, c } {}
CUI_MissionPanel::CUI_MissionPanel(const CUI_MissionPanel& p) : CUI_GenericContainer(p) {}

HRESULT CUI_MissionPanel::Initialize_Prototype() { return __super::Initialize_Prototype(); }
HRESULT CUI_MissionPanel::Initialize(void* pArg) { return __super::Initialize(pArg); }

void CUI_MissionPanel::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);   // ¾Ö´Ï¸ÞÀÌÅÍ(¹Ù¿î½º/ÆäÀÌµå) ÁøÇà

    if (m_eSuccess == ESUCCESS::SHAKING)
    {
        m_fSuccessTimer += fTimeDelta;
        if (m_fSuccessTimer >= m_fShakeDur)
        {
            if (auto it = m_UIPartObjects.find(PART_CAGE); it != m_UIPartObjects.end())
                it->second->Set_Active(false);

            if (auto it = m_UIPartObjects.find(PART_WADDLE); it != m_UIPartObjects.end())
            {
                it->second->Set_Active(true);
                if (auto* pAnim = Get_UIAnimatorCom())
                {
                    UI_BOUNCE_DESC pop{};
                    pop.vDirection = { 0.f, 1.f };
                    pop.fDistance = 14.f;
                    pop.fDuration = 0.25f;
                    pop.fWaveCount = 1.f;
                    pop.fDamping = 1.f;
                    pop.bRestoreOnFinish = true;
                    pAnim->Play_Bounce(PART_WADDLE, pop);

                    UI_FADE_DESC fd{};
                    fd.fFromAlpha = 0.f; fd.fToAlpha = -1.f; fd.fDuration = 0.15f;
                    pAnim->Play_Fade(PART_WADDLE, fd);
                }
            }
            m_eSuccess = ESUCCESS::REVEALED;
        }
    }
}

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
    if (m_eSuccess != ESUCCESS::NONE) return;

    if (auto* pAnim = Get_UIAnimatorCom())
    {
        UI_BOUNCE_DESC shake{};
        shake.vDirection = { 1.f, 0.f };   // ÁÂ¿ì·Î
        shake.fDistance = 6.f;            // ÀÛ°Ô
        shake.fDuration = m_fShakeDur;
        shake.fWaveCount = 12.f;           // ºÎµéºÎµé(¶³¸² È½¼ö)
        shake.fDamping = 1.f;            // Á¡Á¡ Àæ¾Æµê
        shake.bRestoreOnFinish = true;
        pAnim->Play_Bounce(PART_CAGE, shake);
    }

    m_fSuccessTimer = 0.f;
    m_eSuccess = ESUCCESS::SHAKING;
}

void CUI_MissionPanel::On_Deserialized()
{
    __super::On_Deserialized();

    if (!m_pGameInstance_Proxy->Is_EditMode())
        if (auto it = m_UIPartObjects.find(PART_WADDLE); it != m_UIPartObjects.end())
            it->second->Set_Active(false);
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