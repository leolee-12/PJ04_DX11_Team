#pragma once
#include "GameContent_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

// Area/Boss/Cutscene 3캠 공용 셰이크+럼블 로직
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
    }

protected:
    void    Tick_Shake(_float fTimeDelta);     // 파생 Priority_Update 초입에서 호출
    void    Apply_Shake(_matrix& CamWorld);    // 월드행렬 확정 직전에 호출
    HRESULT Ready_ShakeEvents();               // Camera_Shake / Camera_Rumble 구독

protected:
    _float m_fTrauma = { 0.f };
    _float m_fTraumaDecay = { 1.6f };
    _float m_fShakeTime = { 0.f };
    _float m_fRumble = { 0.f };
    _bool  m_bShakeRawTime = { false };

    // 튜닝 (파생 클래스에서 값만 바꾸면 됨)
    _float m_fShakeFreq = { 22.f };
    _float m_fShakeYaw = { XMConvertToRadians(2.2f) };
    _float m_fShakePitch = { XMConvertToRadians(1.8f) };
    _float m_fShakeRoll = { XMConvertToRadians(1.2f) };
    _float m_fShakePos = { 0.12f };
    static constexpr _float DEFAULT_TRAUMA_DECAY = 1.6f;
};

NS_END