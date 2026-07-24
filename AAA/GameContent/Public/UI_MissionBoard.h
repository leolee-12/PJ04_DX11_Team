#pragma once
#include "GameContent_Defines.h"
#include "UI_CoordinatorContainer.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_MissionBoard final : public CUICoordinatorContainer
{
    GENERATED_BODY(CUI_MissionBoard)

    // 에디터 튜닝(보드를 MissionBoard로 저작 후 저장하면 직렬화됨)
    PROPERTY(_float, m_fIntroSlideOffX, L"Intro Slide OffX", L"MissionBoard Intro")
    PROPERTY(_float, m_fIntroSlideDur, L"Intro Slide Dur", L"MissionBoard Intro")
    PROPERTY(_float, m_fIntroStagger, L"Intro Stagger", L"MissionBoard Intro")
    PROPERTY(_float, m_fSuccessGap, L"Success Gap", L"MissionBoard Intro")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_MissionBoard";

private:
    CUI_MissionBoard(ID3D11Device*, ID3D11DeviceContext*);
    CUI_MissionBoard(const CUI_MissionBoard&);
    virtual ~CUI_MissionBoard() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Events() override;
    virtual void    On_Deserialized() override;   // 로드 직후 자식 숨김(런타임)

private:
    void  Hide_AllChildren();
    void  Start_Intro();
    _bool Any_PanelBusy() const;

private:
    enum class EBOARD { IDLE, INTRO, SUCCESS, WAIT_INPUT, DONE };
    EBOARD m_eBoard = { EBOARD::IDLE };
    _float m_fIntroTimer = { 0.f };
    _uint  m_iIntroIdx = { 0 };
    vector<_wstring> m_IntroOrder;
    _float m_fSuccessTimer = { 0.f };
    _uint  m_iSuccessIdx = { 0 };

public:
    static CUI_MissionBoard* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END