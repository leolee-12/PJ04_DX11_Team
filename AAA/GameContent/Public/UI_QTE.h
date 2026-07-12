#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)

class CUI_Image;

// QTE 보조 UI: 조이스틱 아이콘이 랜덤 방향으로 흔들리며 연타를 유도.
// WASD 입력마다 전환이 빨라지고 링이 퍼지며 페이드아웃하는 펄스 연출
class CLIENT_DLL CUI_QTE final : public CUIContainerObject
{
    GENERATED_BODY(CUI_QTE)

    PROPERTY(_float, m_fBaseInterval, L"Base Interval", L"QTE")    // 평상시 전환 간격(초)
    PROPERTY(_float, m_fFastInterval, L"Fast Interval", L"QTE")    // 최고 열 상태 간격(초)
    PROPERTY(_float, m_fHeatPerInput, L"Heat Per Input", L"QTE")   // 입력당 열 상승량
    PROPERTY(_float, m_fHeatDecay, L"Heat Decay", L"QTE")          // 초당 열 감소량
    PROPERTY(_float, m_fRingDuration, L"Ring Duration", L"QTE")    // 링 연출 시간(초)

    PROPERTY(_float, m_fFlashDuration, L"Flash Duration", L"QTE")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_QTE";

private:
    CUI_QTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_QTE(const CUI_QTE& Prototype);
    virtual ~CUI_QTE() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual HRESULT Ready_Events() override;

private:
    void Begin_QTE();
    void On_Pulse();                      // WASD 입력: 열 상승 + 링 발사 + SFX
    void Switch_Stick();                  // 현재와 다른 방향으로 랜덤 전환
    void Show_Stick(_int iIndex);
    void Tick_Ring(_float fTimeDelta);
    void Restore_StickColor();

private:
    static constexpr _int STICK_COUNT = 5;    // C / L / R / U / D

    _int   m_iCurStick = { 0 };
    _float m_fSwitchTimer = { 0.f };
    _float m_fHeat = { 0.f };                 // 0=평상 ~ 1=최고 속도

    static constexpr _int RING_COUNT = 3;

    _bool  m_bRingPlaying[RING_COUNT] = {};
    _float m_fRingTime[RING_COUNT] = {};
    _float m_fRingBaseSize = { 0.f };
    _int   m_iNextRing = { 0 };

    _bool m_bRunning = { false };

    _float4 m_vStickColor2Orig[STICK_COUNT] = {};
    _bool   m_bFlashing = { false };
    _float  m_fFlashTime = { 0.f };

public:
    static CUI_QTE* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END