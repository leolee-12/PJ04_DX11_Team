#include "UI_MissionBoard.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"
#include "Mission_Manager.h"
#include "UI_MissionPanel.h"

namespace {
    // 슬롯 <-> 자식 태그 (0=메인, 1..4=서브)
    static const _tchar* s_ChildTags[CMissionManager::COUNT] =
    { L"Mission_Main", L"Mission_Sub0", L"Mission_Sub1", L"Mission_Sub2", L"Mission_Sub3" };

    constexpr _float ROW_GAP = 120.f;   // 패널 세로 간격 (보드 로컬)
    constexpr _float SUCCESS_GAP = 0.25f;   // 성공 연출 간격(초)
    constexpr _float SLIDE_OFFX = 600.f;   // 슬라이드 시작 오프셋 X
    constexpr _float SLIDE_TIME = 0.4f;    // 슬라이드 시간
}

CUI_MissionBoard::CUI_MissionBoard(ID3D11Device* d, ID3D11DeviceContext* c)
    : CUICoordinatorContainer{ d, c } {
}
CUI_MissionBoard::CUI_MissionBoard(const CUI_MissionBoard& p)
    : CUICoordinatorContainer(p) {
}

HRESULT CUI_MissionBoard::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    // 부트스트랩: 저작 JSON으로 자식이 안 들어왔으면 코드로 구성
    if (Get_ChildOrder().empty())
        Build_Children();

    return S_OK;
}

void CUI_MissionBoard::Build_Children()
{
    // 패널 프로토(Main/Sub/Deco)가 저작 상태로 등록된 레벨 가정
    const _uint lv = Get_LevelIndex();

    // 데코(배경) 먼저 -> 뒤쪽에 그려짐
    Add_Child(lv, L"Proto_UI_MissionDeco", L"Mission_Deco");

    for (_uint i = 0; i < CMissionManager::COUNT; ++i)
    {
        const _wstring proto = (i == CMissionManager::MAIN)
            ? L"Proto_UI_MissionMain" : L"Proto_UI_MissionSub";

        if (FAILED(Add_Child(lv, proto, s_ChildTags[i])))
            continue;

        // 보드 로컬 기준 세로 배치 (그룹행렬이 최종 위치를 잡음)
        if (auto* p = Find_Child(s_ChildTags[i]))
            p->Get_Transform()->Set_State(STATE::POSITION,
                XMVectorSet(0.f, -static_cast<_float>(i) * ROW_GAP, 0.f, 1.f));
    }
}

HRESULT CUI_MissionBoard::Ready_Events()
{
    Subscribe_Event(EventTag::StageClear_SequenceFinished, [this](void*) { Start_Sequence(); });
    return S_OK;
}

void CUI_MissionBoard::Start_Sequence()
{
    if (m_eBoard == EBOARD::APPEARING || m_eBoard == EBOARD::SUCCESS_SEQ)
        return;

    auto* pMgr = CMissionManager::GetInstance();

    // 데이터 주입(이름/성공여부). 매니저는 데이터만.
    for (_uint i = 0; i < CMissionManager::COUNT; ++i)
        if (auto* p = dynamic_cast<CUI_MissionPanel*>(Find_Child(s_ChildTags[i])))
            p->Set_Mission(i == CMissionManager::MAIN, pMgr->Is_Succeeded(i), pMgr->Get_Name(i));

    // 그룹 통째 슬라이드 등장 (자식들이 보드행렬 따라 같이 들어옴)
    Play_SlideIn(_float3{ SLIDE_OFFX, 0.f, 0.f }, SLIDE_TIME);

    m_fTimer = 0.f; m_iSeq = 0; m_eBoard = EBOARD::APPEARING;
}

void CUI_MissionBoard::Update(_float dt)
{
    if (!m_bActive)
        return;

    __super::Update(dt);   // 자식 구동 + 보드 이동/인트로

    switch (m_eBoard)
    {
        case EBOARD::APPEARING:
            if (!Is_Moving())   // 그룹 슬라이드 끝나면 성공 시퀀스로
            {
                m_fTimer = SUCCESS_GAP;
                m_iSeq = 0;
                m_eBoard = EBOARD::SUCCESS_SEQ;
            }
            break;

        case EBOARD::SUCCESS_SEQ:
            m_fTimer += dt;
            if (m_fTimer >= SUCCESS_GAP)
            {
                m_fTimer = 0.f;
                auto* pMgr = CMissionManager::GetInstance();

                // 다음 '성공한' 슬롯까지 전진 (0=메인부터)
                while (m_iSeq < CMissionManager::COUNT && !pMgr->Is_Succeeded(m_iSeq))
                    ++m_iSeq;

                if (m_iSeq < CMissionManager::COUNT)
                {
                    if (auto* p = dynamic_cast<CUI_MissionPanel*>(Find_Child(s_ChildTags[m_iSeq])))
                        p->Play_Success();
                    ++m_iSeq;
                }
                else m_eBoard = EBOARD::DONE;
            }
            break;

        default: break;
    }
}

CUI_MissionBoard* CUI_MissionBoard::Create(ID3D11Device* d, ID3D11DeviceContext* c)
{
    auto* p = new CUI_MissionBoard(d, c);
    if (FAILED(p->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_MissionBoard");
        Safe_Release(p);
    }
    return p;
}

CGameObject* CUI_MissionBoard::Clone(void* pArg)
{
    auto* p = new CUI_MissionBoard(*this);
    if (FAILED(p->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_MissionBoard");
        Safe_Release(p);
    }
    return p;
}

void CUI_MissionBoard::Free()
{
    UnSubscribe_Event(EventTag::StageClear_SequenceFinished);
    __super::Free();
}