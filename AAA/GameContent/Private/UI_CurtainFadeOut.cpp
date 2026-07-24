#include "UI_CurtainFadeOut.h"

CUI_CurtainFadeOut::CUI_CurtainFadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainTexture{ pDevice, pContext }, m_fFadeDelay{ 1.f }, m_fFadeDuration{ 0.5f } {
}
CUI_CurtainFadeOut::CUI_CurtainFadeOut(const CUI_CurtainFadeOut& Prototype)
    : CUI_CurtainTexture(Prototype), m_fFadeDelay{ Prototype.m_fFadeDelay }, m_fFadeDuration{ Prototype.m_fFadeDuration } {
}

HRESULT CUI_CurtainFadeOut::Initialize(void* pArg)
{
    if (pArg)
    {
        auto* p = static_cast<UI_CURTAINFADEOUT_DESC*>(pArg);
        m_fFadeDelay = p->fFadeDelay;
        m_fFadeDuration = p->fFadeDuration;
    }
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_bShowWhileWaiting = true;   // 페이드 전엔 완전 표시(arm 1프레임 숨김 방지)
    m_fAlpha = 1.f; m_fTimer = 0.f;
    return S_OK;
}

void CUI_CurtainFadeOut::Update(_float fTimeDelta)
{
    CUIPartObject::Update(fTimeDelta);   // 트랜스폼 불변(비균일 보존)

    // 런타임 arm -> 즉시 시작(표시 상태로). FadeDelay는 아래 타이머가 담당
    if (m_bArmed) { m_bArmed = false; m_bPlay = true; m_bPrevPlay = false; }

    // 재생 상승엣지(에디터 Play or arm)
    if (m_bPlay && !m_bPrevPlay) { m_fTimer = 0.f; m_fAlpha = 1.f; m_bFinished = false; }
    // Loop만 켠 경우
    if (m_bLoop && !m_bPrevLoop && !m_bPlay) { m_bPlay = true; m_fTimer = 0.f; m_fAlpha = 1.f; m_bFinished = false; }
    m_bPrevPlay = m_bPlay; m_bPrevLoop = m_bLoop;

    if (!m_bPlay) return;

    m_fTimer += fTimeDelta;

    if (m_fTimer < m_fFadeDelay) { m_fAlpha = 1.f; return; }   // 대기: 완전 표시

    _float ft = (m_fFadeDuration > 0.f) ? min((m_fTimer - m_fFadeDelay) / m_fFadeDuration, 1.f) : 1.f;
    m_fAlpha = 1.f - ft;   // 1 -> 0

    if (ft >= 1.f)
    {
        if (m_bLoop) { m_fTimer = 0.f; m_fAlpha = 1.f; }   // 프리뷰용 반복
        else { m_bFinished = true; m_bPlay = false; }
    }
}

void CUI_CurtainFadeOut::Reset()
{
    // __super::Reset()은 Reset_Tranform로 균일 스케일 덮어써서 안 부름
    m_fTimer = 0.f; m_fAlpha = 1.f;
    m_bFinished = false; m_bPlay = false; m_bArmed = false;
}

CUI_CurtainFadeOut* CUI_CurtainFadeOut::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_CurtainFadeOut* p = new CUI_CurtainFadeOut(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_CurtainFadeOut"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_CurtainFadeOut::Clone(void* pArg)
{
    CUI_CurtainFadeOut* p = new CUI_CurtainFadeOut(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_CurtainFadeOut"); Safe_Release(p); }
    return p;
}
void CUI_CurtainFadeOut::Free() { __super::Free(); }