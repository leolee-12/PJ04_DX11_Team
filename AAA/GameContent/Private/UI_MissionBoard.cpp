#include "UI_MissionBoard.h"
#include "GameInstance.h"
#include "GameContent_Events.h"
#include "UI_MissionPanel.h"
#include "Mission_Manager.h"

CUI_MissionBoard::CUI_MissionBoard(ID3D11Device* d, ID3D11DeviceContext* c)
    : CUICoordinatorContainer{ d, c }
{
    m_fIntroSlideOffX = 900.f;   
    m_fIntroSlideDur = 0.30f;    
    m_fIntroStagger = 0.12f;     
    m_fSuccessGap = 1.5f;
}
CUI_MissionBoard::CUI_MissionBoard(const CUI_MissionBoard& p)
    : CUICoordinatorContainer(p)
    , m_fIntroSlideOffX(p.m_fIntroSlideOffX)
    , m_fIntroSlideDur(p.m_fIntroSlideDur)
    , m_fIntroStagger(p.m_fIntroStagger)
    , m_fSuccessGap(p.m_fSuccessGap)
{
}

HRESULT CUI_MissionBoard::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

HRESULT CUI_MissionBoard::Ready_Events()
{
    Subscribe_Event(EventTag::StageClear_SequenceFinished, [this](void*) { Start_Intro(); });
    return S_OK;
}

void CUI_MissionBoard::On_Deserialized()
{
    __super::On_Deserialized();
    // 런타임에선 로드 직후 전부 숨김(이벤트 전까지). 에디터(EditMode)에선 그대로 보임.
    if (m_pGameInstance_Proxy && !m_pGameInstance_Proxy->Is_EditMode())
        Hide_AllChildren();
}

void CUI_MissionBoard::Hide_AllChildren()
{
    for (const _wstring& tag : Get_ChildOrder())
        if (auto* c = Find_Child(tag))
            c->Set_Active(false);
}

void CUI_MissionBoard::Start_Intro()
{
    if (m_eBoard == EBOARD::INTRO || m_eBoard == EBOARD::SUCCESS)
        return;

    m_IntroOrder.clear();
    if (Find_Child(L"Deco"))
        m_IntroOrder.push_back(L"Deco");
    for (const _wstring& tag : Get_ChildOrder())
        if (tag != L"Deco")
            m_IntroOrder.push_back(tag);

    m_fIntroTimer = 0.f;
    m_iIntroIdx = 0;
    m_eBoard = EBOARD::INTRO;
}

void CUI_MissionBoard::Update(_float dt)
{
    if (!m_bActive) return;

    __super::Update(dt);   // 활성 자식 구동(슬라이드 애님 진행)

    static const _tchar* s_SlotTags[CMissionManager::COUNT] =
    { L"MainMission", L"SubMission1", L"SubMission2", L"SubMission3", L"SubMission4" };

    if (m_eBoard == EBOARD::INTRO)
    {
        m_fIntroTimer += dt;
        while (m_iIntroIdx < m_IntroOrder.size() &&
            m_fIntroTimer >= m_iIntroIdx * m_fIntroStagger)
        {
            if (auto* c = Find_Child(m_IntroOrder[m_iIntroIdx]))
            {
                c->Set_Active(true);
                c->Play_SlideIn(_float3{ m_fIntroSlideOffX, 0.f, 0.f }, m_fIntroSlideDur);
            }
            ++m_iIntroIdx;
        }
        if (m_iIntroIdx >= m_IntroOrder.size())
        {
            m_eBoard = EBOARD::SUCCESS;   // 인트로 끝 -> 성공 연출로
            m_fSuccessTimer = 0.f;
            m_iSuccessIdx = 0;
        }
    }
    else if (m_eBoard == EBOARD::SUCCESS)
    {
        m_fSuccessTimer += dt;
        if (m_fSuccessTimer >= m_fSuccessGap)
        {
            m_fSuccessTimer = 0.f;
            auto* pMgr = CMissionManager::GetInstance();

            // 다음 '성공한' 슬롯까지 전진 (0=Main부터)
            while (m_iSuccessIdx < CMissionManager::COUNT && !pMgr->Is_Succeeded(m_iSuccessIdx))
                ++m_iSuccessIdx;

            if (m_iSuccessIdx < CMissionManager::COUNT)
            {
                if (auto* c = dynamic_cast<CUI_MissionPanel*>(Find_Child(s_SlotTags[m_iSuccessIdx])))
                    c->Play_Success();
                ++m_iSuccessIdx;
            }
            else m_eBoard = EBOARD::DONE;
        }
    }
}

CUI_MissionBoard* CUI_MissionBoard::Create(ID3D11Device* d, ID3D11DeviceContext* c)
{
    auto* p = new CUI_MissionBoard(d, c);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_MissionBoard"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_MissionBoard::Clone(void* pArg)
{
    auto* p = new CUI_MissionBoard(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_MissionBoard"); Safe_Release(p); }
    return p;
}
void CUI_MissionBoard::Free()
{
    __super::Free();
}