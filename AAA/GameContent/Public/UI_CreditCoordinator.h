#pragma once

#include "GameContent_Defines.h"
#include "UI_CoordinatorContainer.h"

NS_BEGIN(Client)

class CUIAnimatorCom;

// 엔딩 크레딧 텍스트 코디네이터.
// Credits_Next 신호를 받으면 현재 페이지를 페이드아웃하고 다음 페이지를 페이드인한다.
// 자식(페이지)은 UI 애니메이터를 가진 컨테이너여야 한다 (Proto_UI_GenericContainer 계열)
class CLIENT_DLL CUI_CreditCoordinator final : public CUICoordinatorContainer
{
    GENERATED_BODY(CUI_CreditCoordinator)

    PROPERTY(_float, m_fFadeOutDur, L"Fade Out Dur", L"Credit")
        PROPERTY(_float, m_fFadeInDur, L"Fade In Dur", L"Credit")
        PROPERTY(_float, m_fGap, L"Gap", L"Credit")        // 아웃과 인 사이 빈 시간

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_CreditCoordinator";

private:
    CUI_CreditCoordinator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_CreditCoordinator(const CUI_CreditCoordinator& Prototype);
    virtual ~CUI_CreditCoordinator() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override
    {
        p->strPrototypeTag = PROTOTYPE_TAG;
    }

    _bool Is_Busy() const { return m_ePhase != EPHASE::IDLE; }
    _int  Get_PageIndex() const { return m_iCur; }

protected:
    virtual HRESULT Ready_Events() override;
    virtual void    On_Deserialized() override;

private:
    void On_Next();
    void Begin_FadeOut();
    void Begin_FadeIn();
    void Hide_AllChildren();

    CUIAnimatorCom* Get_ChildAnimator(const _wstring& strChildTag) const;

private:
    enum class EPHASE { IDLE, FADE_OUT, GAP, FADE_IN };

    EPHASE m_ePhase = { EPHASE::IDLE };
    _float m_fTimer = { 0.f };
    _int   m_iCur = { -1 };      // 현재 보이는 페이지. -1 = 아직 시작 안함
    _int   m_iNext = { 0 };

public:
    static CUI_CreditCoordinator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END