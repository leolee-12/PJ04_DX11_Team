#pragma once
#include "GameContent_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

// Area/Boss/Cutscene 3캠 공용 셰이크 + 줌 펀치 베이스
class CLIENT_DLL CCamera_Shakeable abstract : public CCamera
{
protected:
    CCamera_Shakeable(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
        : CCamera(pDevice, pContext) {
    }
    CCamera_Shakeable(const CCamera_Shakeable & Prototype)
        : CCamera(Prototype) {
    }
    virtual ~CCamera_Shakeable() = default;

public:
    void Add_Shake(_float fTrauma, _float fDuration = 0.f, _bool bIgnoreTimeScale = false)
    {
        if (!m_bActive)
            return;

        m_fTrauma = min(1.f, m_fTrauma + fTrauma);
        m_fTraumaDecay = (fDuration > 0.f) ? (m_fTrauma / fDuration) : DEFAULT_TRAUMA_DECAY;
        m_bShakeRawTime = bIgnoreTimeScale;
    }
    void Set_Rumble(_float fLevel)
    {
        if (!m_bActive)
            return;

        m_fRumble = max(0.f, min(1.f, fLevel));
    }
    void Stop_Rumble()
    {
        if (!m_bActive)
            return;

        Add_Shake(m_fRumble);
        m_fRumble = 0.f;
    }

    void Reset_Shake()
    {
        m_fTrauma = 0.f;
        m_fRumble = 0.f;
        m_fShakeTime = 0.f;
        Reset_ZoomPunch();
    }

public:
    // 회피 같은 순간 연출: 확 빠졌다가(In) 잠깐 머물고(Hold) 부드럽게 복귀(Out)
    // fFovAdd : 추가 화각(도) / fDolly : 시선 반대로 물러나는 거리 / fBlur : 가장자리 방사 블러 게인(0~1)
    void Add_ZoomPunch(_float fFovAdd, _float fDolly, _float fBlur = 0.6f,
        _float fInDur = 0.08f, _float fHoldDur = 0.06f, _float fOutDur = 0.3f,
        _bool bIgnoreTimeScale = true);

    void Reset_ZoomPunch()
    {
        m_bZoomActive = false;
        m_fZoomWeight = 0.f;
        m_fZoomTime = 0.f;
        Push_RadialBlur(0.f);
    }

public:
    virtual void Set_Active(_bool b) override;

protected:
    void    Tick_Shake(_float fTimeDelta);     // 파생 Priority_Update 앞에서 호출
    void    Apply_Shake(_matrix& CamWorld);    // 카메라 월드 확정 직후 호출
    void    Apply_ZoomFov();                   // m_fFovy 가산 + Recalculate_ProjMatrix 대체
    HRESULT Ready_ShakeEvents();               // Camera_Shake / Camera_Rumble / Camera_ZoomPunch 구독

private:
    void    Tick_ZoomPunch(_float fTimeDelta);
    _float  Calc_ZoomWeight(_float fTime) const;
    void    Push_RadialBlur(_float fAmount);

protected:
    _float m_fTrauma = { 0.f };
    _float m_fTraumaDecay = { 1.6f };
    _float m_fShakeTime = { 0.f };
    _float m_fRumble = { 0.f };
    _bool  m_bShakeRawTime = { false };

    // 줌 펀치 상태
    _bool  m_bZoomActive = { false };
    _bool  m_bZoomRawTime = { true };
    _float m_fZoomTime = { 0.f };
    _float m_fZoomWeight = { 0.f };
    _float m_fZoomFovAdd = { 0.f };
    _float m_fZoomDolly = { 0.f };
    _float m_fZoomBlur = { 0.f };
    _bool  m_bBlurPushed = { false };
    _float m_fZoomInDur = { 0.08f };
    _float m_fZoomHoldDur = { 0.06f };
    _float m_fZoomOutDur = { 0.3f };

    // 튜닝 (파생 클래스에서 값만 바꾸면 됨)
    _float m_fShakeFreq = { 22.f };
    _float m_fShakeYaw = { XMConvertToRadians(2.2f) };
    _float m_fShakePitch = { XMConvertToRadians(1.8f) };
    _float m_fShakeRoll = { XMConvertToRadians(1.2f) };
    _float m_fShakePos = { 0.12f };
    static constexpr _float DEFAULT_TRAUMA_DECAY = 1.6f;
};

NS_END