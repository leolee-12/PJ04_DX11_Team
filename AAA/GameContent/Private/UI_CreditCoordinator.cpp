#include "UI_CreditCoordinator.h"
#include "GameInstance.h"
#include "GameContent_Events.h"
#include "UIAnimatorCom.h"

CUI_CreditCoordinator::CUI_CreditCoordinator(ID3D11Device* d, ID3D11DeviceContext* c)
    : CUICoordinatorContainer{ d, c }
{
    m_fFadeOutDur = { 0.6f };
    m_fFadeInDur = { 0.6f };
    m_fGap = { 0.15f };
}

CUI_CreditCoordinator::CUI_CreditCoordinator(const CUI_CreditCoordinator& p)
    : CUICoordinatorContainer(p)
    , m_fFadeOutDur(p.m_fFadeOutDur)
    , m_fFadeInDur(p.m_fFadeInDur)
    , m_fGap(p.m_fGap)
{
}

HRESULT CUI_CreditCoordinator::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

HRESULT CUI_CreditCoordinator::Ready_Events()
{
    Subscribe_Event(EventTag::Credits_Next, [this](void*) { On_Next(); });
    return S_OK;
}

void CUI_CreditCoordinator::On_Deserialized()
{
    __super::On_Deserialized();

    // 에디터에서는 전부 보여야 배치가 가능하므로 런타임에서만 숨김
    if (m_pGameInstance_Proxy && !m_pGameInstance_Proxy->Is_EditMode())
    {
        Hide_AllChildren();
        m_iCur = -1;
        m_iNext = 0;
        m_ePhase = EPHASE::IDLE;
    }
}

void CUI_CreditCoordinator::Hide_AllChildren()
{
    for (const _wstring& tag : Get_ChildOrder())
        if (auto* c = Find_Child(tag))
            c->Set_Active(false);
}

CUIAnimatorCom* CUI_CreditCoordinator::Get_ChildAnimator(const _wstring& strChildTag) const
{
    auto* pOwner = dynamic_cast<IUIAnimatorOwner*>(Find_Child(strChildTag));
    return pOwner ? pOwner->Get_UIAnimatorCom() : nullptr;
}

void CUI_CreditCoordinator::On_Next()
{
    if (m_ePhase != EPHASE::IDLE)
        return;                       // 전환 중 신호는 무시

    if (Get_ChildOrder().empty())
        return;

    if (m_iCur < 0)
    {
        m_iNext = 0;                  // 첫 신호는 페이드아웃 없이 바로 첫 장
        Begin_FadeIn();
        return;
    }

    m_iNext = m_iCur + 1;             // 범위를 넘으면 아웃만 하고 끝남
    Begin_FadeOut();
}

void CUI_CreditCoordinator::Begin_FadeOut()
{
    const auto& Order = Get_ChildOrder();
    if (m_iCur < 0 || m_iCur >= (_int)Order.size())
        return;

    if (auto* pAnim = Get_ChildAnimator(Order[m_iCur]))
    {
        UI_FADE_DESC fd{};
        fd.fFromAlpha = -1.f;         // 현재 알파에서 시작
        fd.fToAlpha = 0.f;
        fd.fDuration = max(0.01f, m_fFadeOutDur);
        fd.bRestoreOnFinish = false;
        pAnim->Play_FadeAll(fd);
    }

    m_fTimer = 0.f;
    m_ePhase = EPHASE::FADE_OUT;
}

void CUI_CreditCoordinator::Begin_FadeIn()
{
    const auto& Order = Get_ChildOrder();
    if (m_iNext < 0 || m_iNext >= (_int)Order.size())
    {
        m_ePhase = EPHASE::IDLE;
        return;
    }

    m_iCur = m_iNext;

    if (auto* c = Find_Child(Order[m_iCur]))
        c->Set_Active(true);          // 페이드 도는 동안 계속 켜져 있어야 애니메이터가 돔

    if (auto* pAnim = Get_ChildAnimator(Order[m_iCur]))
    {
        UI_FADE_DESC fd{};
        fd.fFromAlpha = 0.f;          // 즉시 0으로 깔고 시작하므로 첫 프레임 번쩍임 없음
        fd.fToAlpha = 1.f;
        fd.fDuration = max(0.01f, m_fFadeInDur);
        fd.bRestoreOnFinish = false;
        pAnim->Play_FadeAll(fd);
    }

    m_fTimer = 0.f;
    m_ePhase = EPHASE::FADE_IN;
}

void CUI_CreditCoordinator::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    __super::Update(fTimeDelta);      // 활성 자식 갱신 = 자식 애니메이터가 여기서 돔

    if (m_ePhase == EPHASE::IDLE)
        return;

    const auto& Order = Get_ChildOrder();
    m_fTimer += fTimeDelta;

    if (m_ePhase == EPHASE::FADE_OUT)
    {
        if (m_fTimer < m_fFadeOutDur)
            return;

        if (m_iCur >= 0 && m_iCur < (_int)Order.size())
            if (auto* c = Find_Child(Order[m_iCur]))
                c->Set_Active(false);

        m_iCur = -1;
        m_fTimer = 0.f;
        m_ePhase = EPHASE::GAP;
    }
    else if (m_ePhase == EPHASE::GAP)
    {
        if (m_fTimer < m_fGap)
            return;

        if (m_iNext >= (_int)Order.size())
        {
            m_ePhase = EPHASE::IDLE;        // 마지막 장까지 다 넘김
            m_pGameInstance_Proxy->Publish(EventTag::Credits_Finished, nullptr);
            return;
        }

        Begin_FadeIn();
    }
    else if (m_ePhase == EPHASE::FADE_IN)
    {
        if (m_fTimer >= m_fFadeInDur)
            m_ePhase = EPHASE::IDLE;
    }
}

CUI_CreditCoordinator* CUI_CreditCoordinator::Create(ID3D11Device* d, ID3D11DeviceContext* c)
{
    auto* p = new CUI_CreditCoordinator(d, c);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_CreditCoordinator"); Safe_Release(p); }
    return p;
}

CGameObject* CUI_CreditCoordinator::Clone(void* pArg)
{
    auto* p = new CUI_CreditCoordinator(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_CreditCoordinator"); Safe_Release(p); }
    return p;
}

void CUI_CreditCoordinator::Free() { __super::Free(); }