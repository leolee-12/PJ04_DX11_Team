#pragma once
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

// TriggerEvent 받으면 FadeDelay 후 컨테이너 전체를 알파 페이드아웃(UI 애니메이터 Play_FadeAll).
class CLIENT_DLL CUI_UIFadeOut final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_UIFadeOut)

    PROPERTY(_wstring, m_strTriggerEvent, L"TriggerEvent", L"Fade")   // 예: L"FadeIn_Done"
        PROPERTY(_float, m_fFadeDelay, L"FadeDelay", L"Fade")
        PROPERTY(_float, m_fFadeDuration, L"FadeDuration", L"Fade")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_UIFadeOut";

private:
    CUI_UIFadeOut(ID3D11Device*, ID3D11DeviceContext*);
    CUI_UIFadeOut(const CUI_UIFadeOut&);
    virtual ~CUI_UIFadeOut() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    On_Deserialized() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    void Refresh_Subscription();
    void Start_FadeOut();

private:
    _bool     m_bFading = { false };
    _bool     m_bDone = { false };
    SUBHANDLE m_hTrigger{};
    _bool     m_bSubscribed = { false };

public:
    static CUI_UIFadeOut* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END