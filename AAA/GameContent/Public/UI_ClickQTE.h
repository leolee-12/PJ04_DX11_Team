#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)

class CUI_Image;

// 좌클릭 QTE 프롬프트: 버튼이 상시 숨쉬기, 클릭할 때마다 링 파동 + 플래시 + 펀치
class CLIENT_DLL CUI_ClickQTE final : public CUIContainerObject
{
    GENERATED_BODY(CUI_ClickQTE)

    PROPERTY(_float, m_fIdlePeriod, L"Idle Period", L"ClickQTE")       // 숨쉬기 한 주기(초)
        PROPERTY(_float, m_fIdleAmp, L"Idle Amp", L"ClickQTE")             // 숨쉬기 진폭
        PROPERTY(_float, m_fPunchScale, L"Punch Scale", L"ClickQTE")       // 클릭 순간 튀는 양
        PROPERTY(_float, m_fPunchDecay, L"Punch Decay", L"ClickQTE")       // 펀치 감쇠 시간(초)
        PROPERTY(_float, m_fRingDuration, L"Ring Duration", L"ClickQTE")   // 링 수명(초)
        PROPERTY(_float, m_fRingGrow, L"Ring Grow", L"ClickQTE")           // 링 최대 확대 비율
        PROPERTY(_float, m_fFlashDuration, L"Flash Duration", L"ClickQTE")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_ClickQTE";

private:
    CUI_ClickQTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_ClickQTE(const CUI_ClickQTE& Prototype);
    virtual ~CUI_ClickQTE() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual HRESULT Ready_Events() override;

private:
    void Begin_Prompt();
    void End_Prompt();
    void On_Click();
    void Tick_Button(_float fTimeDelta);
    void Tick_Ring(_float fTimeDelta);
    void Restore_Button();

private:
    static constexpr _int RING_COUNT = 3;

    _bool  m_bRunning = { false };

    _float m_fPhase = { 0.f };
    _float m_fPunch = { 0.f };
    _float m_fButtonBaseSize = { 0.f };

    _bool  m_bRingPlaying[RING_COUNT] = {};
    _float m_fRingTime[RING_COUNT] = {};
    _float m_fRingBaseSize = { 0.f };
    _int   m_iNextRing = { 0 };

    _bool  m_bFlashing = { false };
    _float m_fFlashTime = { 0.f };
    _float4 m_vButtonColor2Orig = {};

public:
    static CUI_ClickQTE* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END