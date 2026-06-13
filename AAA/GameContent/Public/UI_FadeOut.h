#pragma once
#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)
class ICurtainPart;

// 별 와이프 페이드아웃 컨테이너. Play()로 일괄 Active+Begin_Delayed,
// 각 파트는 자기 StartDelay 후 재생/끝나면 사라짐. 비루프 파트 전부 Is_Finished면 완료.
class CLIENT_DLL CUI_FadeOut final : public CUIContainerObject
{
    GENERATED_BODY(CUI_FadeOut)

    PROPERTY(_bool, m_bStartHidden, L"StartHidden", L"FadeOut");    // 스폰 시 숨김
    PROPERTY(_bool, m_bAutoStart, L"AutoStart", L"FadeOut");                // 활성 시 자동 Play

    PROPERTY(_bool, m_bLoop, L"Loop", L"FadeOut");


public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_FadeOut";

private:
    CUI_FadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_FadeOut(const CUI_FadeOut& Prototype);
    virtual ~CUI_FadeOut() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

public:
    void    Play();                 // 장면전환 신호: 일괄 시작
    void    Stop();
    _bool   Is_Done()    const { return m_bDone; }
    _bool   Is_Playing() const { return m_bStarted && !m_bDone; }

    void    Reset_Containor();

private:
    void    Collect_Parts();

private:
    vector<ICurtainPart*>       m_Parts;
    _bool   m_bCollected = { false };
    _bool   m_bStarted = { false };
    _bool   m_bDone = { false };

public:
    static CUI_FadeOut* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END