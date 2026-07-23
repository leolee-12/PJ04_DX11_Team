#include "UI_CurtainStamp.h"
#include "Transform.h"

CUI_CurtainStamp::CUI_CurtainStamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainTexture{ pDevice, pContext }, m_fPunchScale{ 1.4f }, m_fEndDelay{ 1.f }, m_fBounce{ 2.5f } {}
CUI_CurtainStamp::CUI_CurtainStamp(const CUI_CurtainStamp& Prototype)
    : CUI_CurtainTexture(Prototype), m_fPunchScale{ Prototype.m_fPunchScale }
    , m_fEndDelay{ Prototype.m_fEndDelay }, m_fBounce{ Prototype.m_fBounce } {}

HRESULT CUI_CurtainStamp::Initialize(void* pArg)
{
    if (pArg)
    {
        auto* p = static_cast<UI_CURTAINSTAMP_DESC*>(pArg);
        m_fPunchScale = p->fPunchScale;
        m_fEndDelay = p->fEndDelay;
        m_fBounce = p->fBounce;
    }
    if (FAILED(__super::Initialize(pArg)))   // 텍스처/컬러/컴포넌트 세팅(트랜스폼은 이후 Deserialize가 비균일로 덮어씀)
        return E_FAIL;

    m_bDisableOnFinish = false;              // 홀드/완료 후에도 계속 표시
    m_bPunchDone = false; m_fHoldAcc = 0.f;
    return S_OK;
}

void CUI_CurtainStamp::Update(_float fTimeDelta)
{
    CUIPartObject::Update(fTimeDelta);

    // 런타임: arm 지연 끝나면 재생 트리거
    if (m_bArmed)
    {
        m_fDelayAcc += fTimeDelta;
        if (m_fDelayAcc < m_fStartDelay) return;
        m_bArmed = false;
        m_bPlay = true; m_bPrevPlay = false;
    }

    // 재생 상승엣지(에디터 Play 토글 or 위 arm): 기준(비균일) 스케일 캡처
    if (m_bPlay && !m_bPrevPlay)
    {
        _float3 s = m_pTransformCom->Get_Scaled();
        m_vBaseScale = { s.x, s.y };
        m_fAccTime = 0.f; m_fHoldAcc = 0.f; m_bPunchDone = false; m_bFinished = false;
    }
    // Loop만 켠 경우도 시작
    if (m_bLoop && !m_bPrevLoop && !m_bPlay)
    {
        m_bPlay = true;
        _float3 s = m_pTransformCom->Get_Scaled();
        m_vBaseScale = { s.x, s.y };
        m_fAccTime = 0.f; m_fHoldAcc = 0.f; m_bPunchDone = false; m_bFinished = false;
    }
    // 정지 하강엣지: 기준 스케일로 복원(다음 재생 때 base 정확히 캡처)
    if (!m_bPlay && m_bPrevPlay && m_vBaseScale.x > 0.f)
        m_pTransformCom->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, 1.f);

    m_bPrevPlay = m_bPlay; m_bPrevLoop = m_bLoop;

    if (!m_bPlay) return;

    // 펀치: PunchScale -> 1.0 (ease-out), 종횡비 보존
    if (!m_bPunchDone)
    {
        m_fAccTime += fTimeDelta;
        _float t = (m_fShrinkDuration > 0.f) ? min(m_fAccTime / m_fShrinkDuration, 1.f) : 1.f;

        _float s = m_fBounce;
        _float u = t - 1.f;
        _float e = 1.f + (s + 1.f) * u * u * u + s * u * u;

        _float f = m_fPunchScale + (1.f - m_fPunchScale) * e;
        m_pTransformCom->Set_Scale(m_vBaseScale.x * f, m_vBaseScale.y * f, 1.f);
        if (t >= 1.f) { m_bPunchDone = true; m_fHoldAcc = 0.f; }
        return;
    }

    // 홀드 -> 완료(비루프) or 재시작(루프)
    m_fHoldAcc += fTimeDelta;
    if (m_fHoldAcc >= m_fEndDelay)
    {
        if (m_bLoop) { m_fAccTime = 0.f; m_fHoldAcc = 0.f; m_bPunchDone = false; }
        else { m_bFinished = true; m_bPlay = false; }
    }
}

void CUI_CurtainStamp::Begin_Delayed()
{
    __super::Begin_Delayed();                // m_bArmed=true, m_fDelayAcc=0, m_bFinished=false
    m_bPunchDone = false; m_fHoldAcc = 0.f;
}
void CUI_CurtainStamp::Reset()
{
    m_fAccTime = 0.f; m_fHoldAcc = 0.f;
    m_bPunchDone = false; m_bFinished = false;
    m_bPlay = false; m_bArmed = false; m_fDelayAcc = 0.f;
    if (m_vBaseScale.x > 0.f)
        m_pTransformCom->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, 1.f);
}

CUI_CurtainStamp* CUI_CurtainStamp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_CurtainStamp* p = new CUI_CurtainStamp(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_CurtainStamp"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_CurtainStamp::Clone(void* pArg)
{
    CUI_CurtainStamp* p = new CUI_CurtainStamp(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_CurtainStamp"); Safe_Release(p); }
    return p;
}
void CUI_CurtainStamp::Free() { __super::Free(); }